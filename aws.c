/*

The program is a simulator for Amazon Web Server (AWS).

It will communicate with clients using TCP connecton to resolve clients' data requests.

After receiving requests from clients, the AWS would consult the backend web servers using UDP

to speed up a large computation task offloaded by clients.

Copyright: Jianfeng(Paul) Wang (USC)

USC ID: 2583-2562-02

Email: jianfenw@usc.edu

*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "aws.h"

int main(int argc, char const *argv[])
{
	int server_tcp_sockfd = 0, client_tcp_sockfd = 0, sa_fd = 0;
	int status, i;
	struct addrinfo hints;
	struct addrinfo *res_a, *res_b, *res_c, *pa, *pb, *pc, *p = NULL;

	struct sockaddr_storage client_addr; // to store the client's address
	socklen_t addr_size = sizeof(client_addr);
	// fd_set readfds;
	signed long long int result = 0, *p_result;
	p_result = (signed long long int *)malloc(sizeof(signed long long int) * 3);
	// int pid = 0;
	int nums_count = 0, bytes = 0;
	int data_length = 0, offset = 0;
	char send_buf[MAXBUFFERSIZE] = "";
	char number_buf[MAXBUFFERSIZE] = "";
	char recv_buf[MAXBUFFERSIZE] = "";
	char operation_buf[MAXLINELENGTH] = "";

	/* create TCP socket */
	if (! (server_tcp_sockfd = tcp_port_listen_socket())){
		fprintf(stderr, "Error: AWS failed to create TCP socket\n");
		exit(1);
	}
	/* create UDP socket */
	if ( !(sa_fd = udp_port_socket()) ){
		fprintf(stderr, "Error: AWS failed to create UDP socket\n");
		exit(1);
	}
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	if ( (status = getaddrinfo("127.0.0.1", SERVER_A_PORT_NUM, &hints, &res_a)) != 0){
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}
	for (pa = res_a; pa != NULL; pa = pa->ai_next){
		if (NULL != pa)
			break;
	}
	if ( (status = getaddrinfo("127.0.0.1", SERVER_B_PORT_NUM, &hints, &res_b)) != 0){
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}
	for (pb = res_b; pb != NULL; pb = pb->ai_next){
		if (NULL != pb)
			break;
	}
	if ( (status = getaddrinfo("127.0.0.1", SERVER_C_PORT_NUM, &hints, &res_c)) != 0){
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}
	for (pc = res_c; pc != NULL; pc = pc->ai_next){
		if (NULL != pc)
			break;
	}

	printf("The AWS is up and running.\n");
	if ( (status = listen(server_tcp_sockfd, BACKLOG) ) == -1){
		fprintf(stderr, "listen error: %s\n", gai_strerror(status));
		exit(1);
	}

	while(1){
		client_tcp_sockfd = accept(server_tcp_sockfd, (struct sockaddr*)&client_addr, &addr_size);
		if (client_tcp_sockfd == -1){
			perror("AWS: accept");
			continue;
		}
		/* Receive the reduction type */
		if ( (bytes = recv(client_tcp_sockfd, recv_buf, sizeof(recv_buf), 0)) == -1){
			perror("AWS: recv");
			close(server_tcp_sockfd);
			close(client_tcp_sockfd);
			exit(1);
		}
		recv_buf[bytes] = '\0';
		strncpy(operation_buf, recv_buf, strlen(recv_buf) + 1);

		/* ACK to client */
		strncpy(send_buf, "ok", 2);
		send_buf[2] = '\0';
		if ((bytes = send(client_tcp_sockfd, send_buf, strlen(send_buf), 0)) == -1){
			perror("AWS: send feedback");
			exit(1);
		}

		/* Receive integers */
		if ( (bytes = recv(client_tcp_sockfd, recv_buf, sizeof(recv_buf), 0)) == -1){
			perror("AWS: recv");
			close(server_tcp_sockfd);
			close(client_tcp_sockfd);
			exit(1);
		}
		recv_buf[bytes] = '\0';
		strncpy(number_buf, recv_buf, strlen(recv_buf) + 1);

		/* ACK to client */
		strncpy(send_buf, "ok", 2);
		send_buf[2] = '\0';
		if ((bytes = send(client_tcp_sockfd, send_buf, strlen(send_buf), 0)) == -1){
			perror("AWS: send feedback");
			exit(1);
		}
		if ( (nums_count = number_count(recv_buf)) % 3 != 0){
			printf("%d\n", nums_count);
			fprintf(stderr, "Error: the number of integers must be 3 * X\n");
			exit(1);
		}
		printf("The AWS has received %d numbers from the client using TCP over port %s\n", nums_count, AWS_TCP_PORT_NUM);

		/* UDP part */
		/* send the reduction type */
		/* send jobs (reduction type and numbers) to Server A, B, C */
		strncpy(send_buf, operation_buf, strlen(operation_buf));
		send_buf[strlen(operation_buf)] = '\n';
		for (i = 1; i <= 3; i++){
			data_length = 0;
			offset = compute_offset(number_buf, nums_count, i, &data_length);
			// printf("Time: %d, offset: %d, data_length: %d\n", i, offset, data_length);
			strncpy(send_buf + 4, number_buf + offset, data_length);
			send_buf[4 + data_length] = '\0';
			
			if (i == 1){
				p = pa;
			}
			else if (i == 2){
				p = pb;
			}
			else{
				p = pc;
			}
			if ( (bytes = sendto(sa_fd, send_buf, strlen(send_buf), 0,
				p->ai_addr, p->ai_addrlen)) == - 1){
				perror("AWS: sendto Server integers");
				exit(1);
			}
			printf("The AWS sent %d numbers to Backend-Server %c\n", nums_count / 3, 'A' + (i - 1));
		}
		
		/* receive the computation result from server A, B, C */
		for (i = 1; i <= 3; i++){
			if ( (bytes = recvfrom(sa_fd, recv_buf, MAXLINELENGTH - 1, 0,
				(struct sockaddr *)&client_addr, &addr_size)) == -1){
				perror("AWS: recvfrom error");
				exit(1);
			}
			recv_buf[bytes] = '\0';
			/* Use getnameinfor to get port number from client_addr */
			char hoststr[NI_MAXHOST];
			char portstr[NI_MAXSERV];
			if (getnameinfo((struct sockaddr *)&client_addr, addr_size, hoststr, sizeof(hoststr),
				portstr, sizeof(portstr), NI_NUMERICHOST | NI_NUMERICSERV) != 0){
				fprintf(stderr, "Error: AWS UDP getnameinfo\n");
			}
			*(p_result + (i - 1)) = atol(recv_buf);
			printf("The AWS received reduction result of %s from Backend-Server %c using UDP over port %s and it is %s\n", operation_buf, 'A' + (i - 1), portstr, recv_buf);
		}
		
		/* AWS does final process and gets the result */
		result = 0;
		if (!strcmp(operation_buf, "MAX")){
			result = max_number(p_result, 3);
		}
		else if (!strcmp(operation_buf, "MIN")){
			result = min_number(p_result, 3);
		}
		else if (!strcmp(operation_buf, "SUM")){
			result = sum_number(p_result, 3);
		}
		else if (!strcmp(operation_buf, "SOS")){
			result = sos_number(p_result, 3);
		}
		else{
			fprintf(stderr, "Error: wrong reduction type %s\n", operation_buf);
			continue;
		}
		/* send result back to the client */
		printf("The AWS has successfully finished the reduction %s: %lli\n", operation_buf, result);
		sprintf(send_buf, "%lli", result);
		if( (bytes = send(client_tcp_sockfd, send_buf, strlen(send_buf), 0)) == -1){
			perror("AWS: TCP sendto error");
			exit(1);
		}
		printf("The AWS has successfully finished sending the reduction value to client.\n");
		close(client_tcp_sockfd);
	}

	free(p_result);
	freeaddrinfo(res_a);
	freeaddrinfo(res_b);
	freeaddrinfo(res_c);
	close(sa_fd);
	close(server_tcp_sockfd);
	return 0;
}

/* 
	Help to split the data_buf into three equal parts.
	It returns the corresponding offset in number_buf.
	It also writes the corresponding string_length into 'int * length'.
*/
int compute_offset(char * number_buf, int nums, int sequence, int * length){
	int start = 0, end = 0, count = 0;
	int start_pos = -1, end_pos = 0, i;
	switch(sequence){
		case 1: 
			start = 1;
			end = nums / 3;
			break;
		case 2: 
			start = nums / 3 + 1;
			end = nums * 2 / 3;
			break;
		case 3:
			start = nums * 2 / 3 + 1;
			end = nums;
			break;
		default:
			printf("Error: wrong sequence number\n");
			break;
	}
	for (i = 0; i < strlen(number_buf); i++){
		if (count + 1 == start && start_pos == -1){
			start_pos = i;
		}
		if (*(number_buf + i) == '\n'){
			count += 1;
		}
		if (count == end && end_pos == 0){
			end_pos = i;
			break;
		}
	}
	*(length) = end_pos - start_pos + 1;
	return start_pos;
}

/* create tcp socket */
int tcp_port_listen_socket(){
	int sockfd, rv;
	int yes = 1;
	struct addrinfo hints;
	struct addrinfo *res, *p;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ( (rv = getaddrinfo(NULL, AWS_TCP_PORT_NUM, &hints, &res) ) != 0){
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rv));
		exit(1);
	}
	// loop through all possible results to bind to the first available one
	for (p = res; p != NULL; p = p->ai_next){
		if ( (sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1 ){
			perror("Error: AWS TCP socket");
			continue;
		}
		if ( (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)))  == -1){
			perror("Error: AWS setsockopt");
			exit(1);
		}
		if ( (bind(sockfd, p->ai_addr, p->ai_addrlen) ) == -1){
			close(sockfd);
			perror("Error: AWS bind");
			exit(1);
		}
		break;
	}
	if (NULL == p){
		fprintf(stderr, "AWS: failed to bind to a port number\n");
		exit(1);
	}
	freeaddrinfo(res);
	res = NULL;
	return sockfd;
}

/* create udp socket */
int udp_port_socket(){
	int sockfd, rv;
	struct addrinfo hints;
	struct addrinfo *res, *p;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	if ( (rv = getaddrinfo("127.0.0.1", AWS_UDP_PORT_NUM, &hints, &res)) != 0){
		fprintf(stderr, "Error: AWS getaddrinfo failed: %s\n", gai_strerror(rv));
		exit(1);
	}
	for (p = res; p != NULL; p = p->ai_next){
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("Error: AWS UDP socket");
			continue;
		}
		break;
	}
	if (NULL == p){
		fprintf(stderr, "Error: AWS failed to create UDP socket\n");
		exit(1);
	}
	freeaddrinfo(res);
	res = NULL;
	return sockfd;
}

/* get the number of integers from the received string */
int number_count(char * number_buf){
	int i, nums = 0;
	for (i = 0; i < strlen(number_buf); i++){
		if (*(number_buf + i) == '\n')
			nums += 1;
	}
	return nums;
}

/* Do final computation: MAX, MIN, SUM, SOS */
signed long long int max_number(signed long long int *number_list, int list_length){
	int i;
	signed long int current_max = 0, current_num;
	for(i = 0; i < list_length; i++){
		current_num = *(number_list + i);
		if (current_num > current_max){
			current_max = current_num;
		}
	}
	return current_max;
}

signed long long int min_number(signed long long int* number_list, int list_length){
	int i;
	signed long int current_min = 2147483647, current_num;
	for(i = 0; i < list_length; i++){
		current_num = *(number_list + i);
		if (current_num < current_min){
			current_min = current_num;
		}
	}
	return current_min;
}

signed long long int sum_number(signed long long int * number_list, int list_length){
	int i;
	signed long int sum = 0, current_num;
	for (i = 0; i < list_length; i++){
		current_num = *(number_list + i);
		sum += current_num;
	}
	return sum;
}

signed long long int sos_number(signed long long int * number_list, int list_length){
	int i;
	signed long int sos = 0, current_num;
	for (i = 0; i < list_length; i++){
		current_num = *(number_list + i);
		sos += current_num;
	}
	return sos;
}

