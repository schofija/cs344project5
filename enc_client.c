/*
	enc_client.c
	arguments: $ enc_client plaintext key port
	
	plaintext and key are sent to enc_server@port
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  /* ssize_t */
#include <sys/socket.h> /* send(),recv() */
#include <netdb.h>      /* gethostbyname() */

#ifndef HOSTNAME /* use localhost unless otherwise specified */
	#define HOSTNAME "localhost"
#endif

#ifndef FILE_SIZE /* max buffer length */
	#define FILE_SIZE 100000
#endif

#ifndef ENC_TOKEN /* used to notify a server that it is speaking to enc_client */
	#define ENC_TOKEN "e\n"
#endif

#ifndef BAD_SERVER_MESSAGE /* used to notify client it has connected to the wrong server */
	#define BAD_SERVER_MESSAGE "x"
#endif

void sendfile(int, FILE*, FILE*);
int sendall(int, char*, int);
void error(const char*);
void setupAddressStruct(struct sockaddr_in*, int, char*);

/* Validates contents of a text file 
 * If text file contains characters not in the A-Z, ' ' scope,
 * An error message is printed and the program exits
 * */
void validatetext(char* text)
{
	unsigned int i = 0;
	while(text[i] != '\n')
	{
		if(text[i] < 65 || text[i] > 90)
		{
			if(text[i] != 32)
			{
				fprintf(stderr, "Invalid text file\n");
				exit(1);
			}
		}
		i++;
	}
}

int main(int argc, char *argv[]) 
{
	if(argc < 4) /* Check arguments */
	{
		fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); 
		exit(0); 
	}

	/* Open plaintext file in read-only mode */
	FILE* plaintext;
	char* plaintext_filename = argv[1];
	if( (plaintext = fopen(plaintext_filename, "r")) == NULL )
		error("CLIENT: Error opening plaintext file\n");
	
	/* Open key file in read-only mode */
	FILE* key;
	char* key_filename = argv[2];
	if( (key = fopen(key_filename, "r")) == NULL )
		error("CLIENT: Error opening key file\n");
	
	int portNumber = atoi(argv[3]); /* Setting port number */
	
	struct sockaddr_in serverAddress;
	
	/* Create socket */
	int socketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if (socketFD < 0)
		error("CLIENT: ERROR opening socket");
 
	/* Set up the server address struct */
	setupAddressStruct(&serverAddress, portNumber, HOSTNAME);

	/* Connect to server */
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
		error("CLIENT: ERROR connecting");

	char buffer[FILE_SIZE] = {0};
	int charsRead;
	
	sendfile(socketFD, plaintext, key); /* Sending plaintext and key to server */

	memset(buffer, '\0', FILE_SIZE);
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
	if (charsRead < 0)
		error("CLIENT: ERROR reading from socket");

	if(strcmp(buffer, BAD_SERVER_MESSAGE) == 0)
	{
		fprintf(stderr, "%s", "Server rejection!\n");
		close(socketFD);
		exit(2);
	}
	else
	{
		printf("%s", buffer);
		close(socketFD); 
		return 0;
	}
return 0;
}

/* 
	Sends a file to the specified socket
	Return's number of characters written.
	
	This function takes code from Nikhil Tomar's
	"File Transfer using TCP Socket in C" article,
	which can be found @ idiotdeveloper.com
*/
void sendfile(int socketFD, FILE* plaintext, FILE* key)
{
	char message[(FILE_SIZE * 2) + 1] = {0};
	
	char buffer1[FILE_SIZE];
	fgets(buffer1, FILE_SIZE, plaintext);

	validatetext(buffer1); /*Make sure plaintext has no bad-chars*/
	
	char buffer2[FILE_SIZE];
	fgets(buffer2, FILE_SIZE, key);
	
	if(strlen(buffer1) > strlen(buffer2))
	{
		fprintf(stderr, "Key is too short!\n");
		exit(1);
	}
	
	strcat(message, ENC_TOKEN); /* "e\n" */
	strcat(message, buffer1);	/* "[plaintext]\n" */
	strcat(message, buffer2);	/* "key]\n" */

	if(sendall(socketFD, message, strlen(message)) == -1) /* Sending data */
		error("CLIENT: sendall() failed!\n");
}

/*
	Function to handle partial sends.
	
	This function was heavily inspired by:
	http://beej.us/guide/bgnet/html/#sendall
*/
int sendall(int socketFD, char* buffer, int len)
{
	int sent = 0;
	int bytesleft = len;
	int n;
	
	while(sent < len)
	{
		if( (n = send(socketFD, buffer + sent, bytesleft, 0)) == -1 )
			error("CLIENT: Error sending file.\n");
		sent += n;
		bytesleft -= n;
	}
	
	len = sent; // return number actually sent here
	
	return n==-1?-1:0; // return -1 on failure, 0 on success
}

// Error function used for reporting issues
void error(const char *msg) 
{ 
  perror(msg); 
  exit(0); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname)
{
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}
