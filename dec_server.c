#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef FILE_SIZE /* max buffer length */
	#define FILE_SIZE 4096
#endif

char* getfile(int, char[], int);
void error(const char*);
void setupAddressStruct(struct sockaddr_in*, int);

void decrypt(char* text, char* decryptedtext)
{
	char *saveptr = NULL;
	char *plaintext = strtok_r(text, "\n", &saveptr);
	char *key = strtok_r(NULL, "\n", &saveptr);
	
	for(int i = 0; i < strlen(plaintext); i++)
	{	
		int keyval;
		if(key[i] == 32) 	/* Setting space to 26 */
			keyval = 26;
		else				/* Setting A-Z to 0-25 */
			keyval = key[i] - 65;
		
		int plaintextval = plaintext[i] - 65;
			
		int cipher = plaintextval - keyval;
		if(cipher < 0)
			cipher += 27;
		cipher %= 27;
		//printf("encr_char: %i, key: %i, cipher: %i\n", plaintextval, keyval, cipher);
		
		/* Convert back to ascii code */
		if(cipher == 26)
			cipher = 32;
		else
			cipher += 65;
		decryptedtext[i] = cipher;
	}
}

int main(int argc, char *argv[])
{
  int connectionSocket, charsRead;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    exit(1);
  } 
  
  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) 
    error("ERROR opening socket");

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    error("ERROR on binding");

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5); 
  
  // Accept a connection, blocking if one is not available until one connects
  while(1)
  {
		// Accept the connection request which creates a connection socket
		connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
		if (connectionSocket < 0)
		  error("ERROR on accept");
		//printf("SERVER: Connected to client running at host %d port %d\n", ntohs(clientAddress.sin_addr.s_addr), ntohs(clientAddress.sin_port));

		char buffer[FILE_SIZE] = {0};
		char *text = getfile(connectionSocket, buffer, FILE_SIZE - 1);
		printf("text: %s\n", text);

		char ciphertext[FILE_SIZE] = {0};
		decrypt(text, ciphertext);
		
		/* Send ciphertext to client */
		charsRead = send(connectionSocket, ciphertext, strlen(ciphertext), 0); 
		
		if (charsRead < 0)
		  error("ERROR writing to socket");

		// Close the connection socket for this client
		close(connectionSocket); 
		//memset(buffer, '\0', FILE_SIZE);
		//free(text);
	  }
	  // Close the listening socket
	  close(listenSocket); 
	  return 0;
}

char* getfile(int connectionSocket, char buffer[], int len)
{
	char* file_contents = malloc(sizeof buffer);
	int charsRead;
	
	if (charsRead = recv(connectionSocket, buffer, len, 0) == -1 )
		error("ERROR reading from socket\n");
	
	strcpy(file_contents, buffer);
	
	memset(buffer, '\0', FILE_SIZE);
	return file_contents;
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
