# miniAWS

The system is the final project of EE450@USC.

In this project, I implement a simple model of computational offloading where a single client offloads some computation to a server which in turn distributes the load over 3 Backend-servers. The system consists of one major server called the 'AWS' and three Backend-servers called 'Server A', 'Server B', 'Server C'. 

In each round, the client program connects to AWS through socket functions. The client gets a reduction type from the commandline input, checks the reduction type and then sends the reduction type to the AWS. The client also reads 3*n signed long integers from a given file 'nums.csv' and sends integers to the AWS through the same TCP connection. If the AWS receives reduction type and integers successfully, the client should get a "ok" message to indicate the data transmission succeeds. Finally, the client waits for the result from AWS, prints the result and then terminates itself.

The AWS and the three Backend servers are always on and waiting for potential computation requests. From the AWS's point of view, the AWS has a TCP socket listening on the AWS TCP port for any incoming clients' connection requests. Once a client tries to connect with it, the AWS will create a new TCP socket which is dedicated for the data transmission. Then, AWS will receive a computation request (reduction type: MAX, MIN, SUM, SOS), and receives a bunch of integers. AWS will split the data into three equal parts, sends each part to one of Backend servers, and waits for their responses. Once AWS receives the results from three Backend-servers, the AWS will do final computation, get the final result, send it back to the client and close the TCP connection.

From the Backend server's point of view, server A, B, and C are always waiting for a computation request (reduction type: MAX, MIN, SUM, SOS) and the corresponding data sent from AWS. After receiving the 'computation task', server A, B, C will process the data, get the result, send the result back to AWS and enter the next round (waiting-receiving-processing-sending).

The client and AWS communicates over a TCP connection while the communication between AWS and Backend-servers is over a UDP connection.


HOW TO RUN THE CODE?
(1) compile the code:
type 'make clean' first to remove all unrelated files;
type 'make' or 'make all' to compile code to get executables (serverX_exe, aws_exe, client_exe)

(2) make Backend-servers run:

(The sequence of the three commands does not matter)
type 'make serverA' to run serverA_exe;
type 'make serverB' to run serverB_exe;
type 'make serverC' to run serverC_exe;

Make sure that the three Backend-servers are running properly before you go further. You should get a printout like this:
The Server X is up and running using UDP on port 2XXXX
......

(3) make AWS server run:

type 'make aws' to run aws_exe;

Make sure that the AWS are running properly before you run the client program. You should get a printout like this:
The AWS is up and running.
......

(4) make client program run:

type './client xxx' to run client;
Note: xxx is one of the reduction types in {max, min, sum, sos}. If you do not input the correct reduction type, you will receive a error message. The client program will also be terminated.

The correct output should look like the following:
The client is up and running.
The client has sent the reduction type xxx to AWS.
The client has sent ???? numbers to AWS.
......