#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h> // for waitpid

#ifndef FILE_SIZE /* max buffer length */
	#define FILE_SIZE 200002
#endif

void error(const char*);
void setupAddressStruct(struct sockaddr_in*, int);

int main(int argc, char *argv[])
{
	int connectionSocket;
	struct sockaddr_in serverAddress, clientAddress;
	socklen_t sizeOfClientInfo = sizeof(clientAddress);

	if (argc < 2) 
	{   /* Check usage & args */
		fprintf(stderr,"USAGE: %s port\n", argv[0]); 
		exit(1);
	} 
  
	int listenSocket = socket(AF_INET, SOCK_STREAM, 0); /* Create the socket that will listen for connections */
	if (listenSocket < 0) 
		error("ERROR opening socket");

	setupAddressStruct(&serverAddress, atoi(argv[1])); /* Set up the address struct for the server socket */

	if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) /* Associate the socket to the port */
		error("ERROR on binding");

	listen(listenSocket, 5); /* Listening for connections. Queue up to 5 */
  
	// Accept a connection, blocking if one is not available until one connects
	while(1)
	{
		// Accept the connection request which creates a connection socket
		connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
		if (connectionSocket < 0)
			error("ERROR on accept");
		
		int len = snprintf(NULL, 0, "%d", connectionSocket); /* trick to get length using snprintf(), 
																courtesy of user2622016 on stackoverflow */
		char connectionSocketStr[len + 1]; /* one extra char for NULL terminator*/
		sprintf(connectionSocketStr, "%d", connectionSocket);
		
		char* args[] = {"./encrypt", connectionSocketStr, NULL};
		int childstatus;
		
		pid_t spawnpid = fork(); /* forking new proc */
		
		switch(spawnpid)
		{
			case -1:	/* something went wrong... :( */
				perror("fork()\n");
				exit(1);
				break;
			case 0:		/* child process */
				execv(args[0], args);
				perror("execv\n");
				exit(2);
				break;
			default: 	/* parent process */
				close(connectionSocket);
				spawnpid = waitpid(spawnpid, &childstatus, WNOHANG);
				break;
		}	
	}
	  // Close the listening socket
	  close(listenSocket); 
	  return 0;
}

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
} 

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, int portNumber)
{
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}
