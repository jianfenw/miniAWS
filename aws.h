/*

Author: Jianfeng Wang (jianfenw@usc.edu)
USC ID: 2583-2562-02

*/

#ifndef _AWS_H_
#define _AWS_H_

#ifndef USC_ID
#define USC_ID 202
#endif /* ~USC_ID */

#ifndef LOCAL_ADDR
#define LOCAL_ADDR "127.0.0.1"
#endif /* ~LOCAL_ADDR */

#ifndef SERVER_A_PORT_NUM
#define SERVER_A_PORT_NUM "21202"
#define SERVER_B_PORT_NUM "22202"
#define SERVER_C_PORT_NUM "23202"
#define AWS_UDP_PORT_NUM "24202"
#define AWS_TCP_PORT_NUM "25202"
#endif /* ~SERVER_#_PORT_NUM */

#ifndef max
#define max(A, B) ( ((A) > (B)) ? (A) : (B))
#define min(A, B) ( ((A) > (B)) ? (B) : (A))
#endif /* ~max */

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif /* ~TRUE */

#ifndef BACKLOG
#define BACKLOG 10
#endif /* ~BACKLOG */

#ifndef MAXBUFFERSIZE
#define MAXBUFFERSIZE 8 * 3000
#endif

#ifndef MAXLINELENGTH
#define MAXLINELENGTH 256
#endif /* ~MAXLINELENGTH */

int compute_offset(char * number_buf, int nums, int sequence, int * length);
int number_count(char * number_buf);
int tcp_port_listen_socket();
int udp_port_socket();

signed long long int max_number(signed long long int*, int);
signed long long int min_number(signed long long int* number_list, int list_length);
signed long long int sum_number(signed long long int * number_list, int list_length);
signed long long int sos_number(signed long long int * number_list, int list_length);

#endif /* _AWS_H */
