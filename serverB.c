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

#include "serverA.h"

int main(int argc, char const *argv[])
{
	int backend_udp_sockfd = 0;
	// struct addrinfo hints, *res, *p;
	struct sockaddr_storage client_addr;
	socklen_t addr_size = sizeof(client_addr);

	int bytes = 0;
	char send_buf[MAXBUFFERSIZE] = "";
	char recv_buf[MAXBUFFERSIZE] = "";
	char operation_buf[MAXLINELENGTH] = "";
	signed long int *number_buffer = NULL;
	number_buffer = (signed long int*)malloc(sizeof(signed long int) * 1000);
	int nums_count = 0;
	signed long long int result = 0;

	/* create UDP socket on port number "21000" */
	if (!(backend_udp_sockfd = udp_port_socket())){
		fprintf(stderr, "Error: Server B failed to create UDP socket\n");
		exit(1);
	}
	printf("The Server B is up and running using UDP on port %s\n", SERVER_B_PORT_NUM);
	/* 
		In each round, the server will receive data from AWS and process it.
		The data consists of a reduction type and many integers.
	   	Then the server will send the result back to AWS using the same port number.
	*/ 
	while(1){
		nums_count = 0;	
		if ((bytes = recvfrom(backend_udp_sockfd, recv_buf, sizeof(recv_buf), 0, 
			(struct sockaddr *)&client_addr, &addr_size)) == -1){
			perror("server B: recvfrom");
			exit(1);
		}
		recv_buf[bytes] = '\0';
		/* get the reduction type ... */
		strncpy(operation_buf, recv_buf, 3);
		operation_buf[3] = '\0';

		nums_count = process_number_buffer(recv_buf + 4, number_buffer);
		printf("The Server B has received %d numbers\n", nums_count);
		/* do computation ... */
		result = 0;
		if (!strcmp(operation_buf, "MAX")){
			result = max_number(number_buffer, nums_count);
		}
		else if (!strcmp(operation_buf, "MIN")){
			result = min_number(number_buffer, nums_count);
		}
		else if (!strcmp(operation_buf, "SUM")){
			result = sum_number(number_buffer, nums_count);
		}
		else if (!strcmp(operation_buf, "SOS")){
			result = sos_number(number_buffer, nums_count);
		}
		else{
			fprintf(stderr, "Error: wrong reduction type %s\n", operation_buf);
			continue;
		}
		printf("The Server B has successfully finished the reduction %s: %lli\n", operation_buf, result);
		sprintf(send_buf, "%lli", result);
		if ((bytes = sendto(backend_udp_sockfd, send_buf, strlen(send_buf), 0,
			(struct sockaddr *)&client_addr, addr_size)) == -1){
			perror("ServerB: sendto error");
			exit(1);
		}
		printf("The Server B has successfully finished sending the reduction value to AWS server\n");
	}

	close(backend_udp_sockfd);
	free(number_buffer);
	return 0;
}

/*
	create a UDP socket waiting at the corresponding port
	The server X should ues the socket to communicate with AWS
*/
int udp_port_socket(){
	int sockfd, rv;
	struct addrinfo hints;
	struct addrinfo *res, *p;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	if ( (rv = getaddrinfo(NULL, SERVER_B_PORT_NUM, &hints, &res)) != 0){
		fprintf(stderr, "Error: AWS getaddrinfo failed: %s\n", gai_strerror(rv));
		exit(1);
	}
	for (p = res; p != NULL; p = p->ai_next){
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("Error: Server B UDP socket");
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("Error: Server B bind");
			continue;
		}
		break;
	}
	if (NULL == p){
		fprintf(stderr, "Error: Server B failed to create UDP socket\n");
		exit(1);
	}
	freeaddrinfo(res);
	res = NULL;
	return sockfd;
}

/* 
	convert the string received from the UDP connection into signed long int list
*/
int process_number_buffer(char * number_buf, signed long int *number_list){
	char *start_ptr = number_buf;
	char *tab_ptr = strchr(start_ptr, '\n');
	int nums_count = 0;
	while(tab_ptr != NULL){
		*(tab_ptr++) = '\0';
		*(number_list + nums_count) = atol(start_ptr);
		nums_count += 1;
		start_ptr = tab_ptr;
		tab_ptr = strchr(start_ptr, '\n');
	}
	return nums_count;
}

/*
	do computation based on the reduction type and signed long int list
*/
signed long long int max_number(signed long int* number_list, int list_length){
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

signed long long int min_number(signed long int* number_list, int list_length){
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

signed long long int sum_number(signed long int * number_list, int list_length){
	int i;
	signed long int sum = 0, current_num;
	for (i = 0; i < list_length; i++){
		current_num = *(number_list + i);
		sum += current_num;
	}
	return sum;
}

signed long long int sos_number(signed long int * number_list, int list_length){
	int i;
	signed long int sos = 0, current_num;
	for (i = 0; i < list_length; i++){
		current_num = *(number_list + i);
		sos += current_num * current_num;
	}
	return sos;
}

