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

#ifndef MAXBUFFERSIZE
#define MAXBUFFERSIZE 8 * 3000
#endif

#ifndef MAXLINELENGTH
#define MAXLINELENGTH 256
#endif /* ~MAXLINELENGTH */

#ifndef BACKLOG
#define BACKLOG 10
#endif /* ~BACKLOG */ 

char reduction_type[4][4] = {"MAX", "MIN", "SUM", "SOS"};
int read_file(char *file_name, char *send_buffer, int *number_count);
int connect_to_aws();

#endif /* _AWS_H */
