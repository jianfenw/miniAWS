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

#include "client.h"

/*
Phase 1:
Server:
boot up and print boot-up message
listen on the appropriate port for incoming packets/ connections

Client:
boot up and print boot-up message
take an input argument from the command line {min, max, sum, sos}
establish a TCP connection with AWS
-> send the <function name> to AWS, and print a message
-> read all integers from nums.csv and send them to AWS over the same TCP conection
-> print the number of integers sent to AWS
*/

int main(int argc, char const *argv[])
{
	int client_tcp_sockfd = 0;
	int bytes = 0;
	char send_buf[MAXBUFFERSIZE], recv_buf[MAXBUFFERSIZE]; // send buf and recv buf for tcp connection
	char operation_buf[MAXLINELENGTH] = "";
	memset(send_buf, 0, sizeof(char) * MAXBUFFERSIZE);
	memset(recv_buf, 0, sizeof(char) * MAXBUFFERSIZE);
	
	char *number_file = "nums.csv";
	int nums_count = 0;
	/* check the reduction type */
	if (argc != 2){
		fprintf(stderr, "Error: the command line needs 'reduction type' parameter\n./client {max, min, sum, sos}\n");
		exit(1);
	}
	if (strcmp(argv[1], "max") == 0){
		strncpy(operation_buf, "MAX", 3);
		operation_buf[3] = '\0';
	}
	else if (strcmp(argv[1], "min") == 0){
		strncpy(operation_buf, "MIN", 3);
		operation_buf[3] = '\0';
	}
	else if (strcmp(argv[1], "sum") == 0){
		strncpy(operation_buf, "SUM", 3);
		operation_buf[3] = '\0';
	}
	else if (strcmp(argv[1], "sos") == 0){
		strncpy(operation_buf, "SOS", 3);
		operation_buf[3] = '\0';
	}
	else{
		fprintf(stderr, "Error: wrong reduction type\n./client {max, min, sum, sos}\n");
		exit(1);
	}
	printf("The client is up and running.\n");

	/*
		After booting up, the client establishes a TCP connection with AWS.
	*/
	if ( !( client_tcp_sockfd = connect_to_aws()) ){
		fprintf(stderr, "Error: client failed to connect with AWS\n");
		exit(1);
	}
	/* 
		The client sents the reduction type: max, min, sum, sos 
		If it is successful, the client should print a message
	*/
	strncpy( send_buf, operation_buf, strlen(operation_buf));
	send_buf[ strlen(operation_buf) ] = '\0';
	if ( (bytes = send(client_tcp_sockfd, send_buf, strlen(send_buf), 0)) == -1){
		perror("client: send");
		exit(1);
	}
	if ((bytes = recv(client_tcp_sockfd, recv_buf, MAXLINELENGTH - 1, 0)) == -1){
		perror("client: recv");
		exit(1);
	}
	recv_buf[bytes] = '\0';
	if (strcmp(recv_buf, "ok") != 0){
		perror("Error: the AWS does not receive the numbers");
		exit(1);
	}
	printf("The client has sent the reduction type %s to AWS.\n", operation_buf);

	/* 
		read file and transmit all numbers over the same TCP connection
		After successfully sending the integers, the client should print the number
		of integers sent to AWS.
		(Phase 1 ends)
	*/
	if (!read_file(number_file, send_buf, &nums_count)){
		fprintf(stderr, "Error: cannot read the input file\n");
		exit(1);
	}
	// printf("%s%d", send_buf, nums_count);
	if ( (bytes = send(client_tcp_sockfd, send_buf, strlen(send_buf), 0)) == -1){
			perror("client: send");
			exit(1);
	}
	if ((bytes = recv(client_tcp_sockfd, recv_buf, MAXLINELENGTH - 1, 0)) == -1){
		perror("client: recv");
		exit(1);
	}
	recv_buf[bytes] = '\0';
	if (strcmp(recv_buf, "ok") != 0){
		perror("Error: the AWS does not receive the numbers");
		exit(1);
	}
	printf("The client has sent %d numbers to AWS.\n", nums_count);
	
	if ((bytes = recv(client_tcp_sockfd, recv_buf, MAXLINELENGTH - 1, 0)) == -1){
		perror("Error: client recv result");
		exit(1);
	}
	recv_buf[bytes] = '\0';
	printf("The client has received reduction %s: %s\n", operation_buf, recv_buf);
	close(client_tcp_sockfd);
	return 0;
}

/* 
	Read the 'file_name' file;
	Parse the integers from the file into send_buffer and
	write the number of integers into 'int *number_count'
*/
int read_file(char *file_name, char *send_buffer, int *number_count){
	// the file descriptor to read nums.csv
	FILE *fp = NULL;
	int count = 0;
	char buf[MAXLINELENGTH];
	memset(buf, 0, sizeof(buf));
	memset(send_buffer, 0, sizeof(char) * MAXBUFFERSIZE);
	fp = fopen(file_name, "r");
	if (NULL == fp){
		fprintf(stderr, "Error: cannot open file '%s'\n", file_name);
		perror(file_name);
		exit(1);
	}
	int offset = 0;
	size_t temp_len = 0;
	while(fgets(buf, sizeof(buf), fp) != NULL){
		//printf("%s, %d, %d\n", buf, sizeof(buf), strlen(buf));
		if (buf[strlen(buf) - 1] != '\n'){
			temp_len = strlen(buf);
			buf[temp_len] = '\n';
			buf[temp_len + 1] = '\0';
			// printf("%d %s\n", count, buf);
		}
		strncpy(send_buffer + offset, buf, strlen(buf));
		count += 1;
		offset += strlen(buf);
	}
	send_buffer[offset] = '\0';
	fclose(fp);
	*number_count = count;
	return TRUE;
}

/* 
	create the client TCP socket
	The client should use the socket to communicate with AWS throughout this program
*/
int connect_to_aws(){
	int sockfd, rv;
	struct addrinfo hints;
	struct addrinfo *res, *p;
	// char s[INET6_ADDRSTRLEN], ipstr[INET6_ADDRSTRLEN];
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ( (rv = getaddrinfo("localhost", AWS_TCP_PORT_NUM, &hints, &res)) != 0){
		fprintf(stderr, "Error: client getaddrinfo failed: %s\n", gai_strerror(rv));
		exit(1);
	}
	for (p = res; p != NULL; p = p->ai_next){
		void *addr;
		char *ipver;
		if (p->ai_family == AF_INET){	// ipv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = "IPV4";
		}
		else{ // ipv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = "IPV6";
		}
		if ( (sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("Error: client failed to create socket");
			continue;
		}
		if ( (connect(sockfd, p->ai_addr, p->ai_addrlen)) == -1){
			close(sockfd);
			perror("client: connection failed");
			exit(1);
		}
		break;
	}
	if (NULL == p){
		fprintf(stderr, "client: failed to connect with the server\n");
		exit(1);
	}
	freeaddrinfo(res);
	res = NULL;
	return sockfd;
}
