/*
	encrypt.c
	usage: ./encrypt connectionSocket
	
	Ran by enc_server after a fork()
	This program:	1. gets the text sent from the client (plaintext & key)
					2. encrypts plaintext
					3. sends the ciphertext back to the client
					4. closes the connection socket
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef FILE_SIZE /* max buffer length */
	#define FILE_SIZE 200002
#endif

#ifndef ENC_VALIDATION /* validates connection is coming from enc_client */
	#define ENC_VALIDATION "e"
#endif

#ifndef BAD_CLIENT_MESSAGE
	#define BAD_CLIENT_MESSAGE "x"
#endif

#ifndef BAD_KEY
	#define BAD_KEY 'k'
#endif

char* getfile(int, int); //socketFD, buffer, len
void encrypt(char*, char*); /* 	usage: text, ciphertext, 
								encrypted string written to ciphertext */
								
int sendall(int socketFD, char* buffer, int len)
{
	int sent = 0;
	int bytesleft = len;
	int n;
	
	while(sent < len)
	{
		if( (n = send(socketFD, buffer + sent, bytesleft, 0)) == -1 )
		{
			perror("ERROR sending file!\n");
			exit(1);
		}
		sent += n;
		bytesleft -= n;
	}
	
	len = sent; // return number actually sent here
	
	return n==-1?-1:0; // return -1 on failure, 0 on success
}
								
int main(int argc, char* argv[])
{
	
	if(argc != 2)
	{	/* check usage */
		fprintf(stderr,"USAGE: %s connectionSocket\n", argv[0]); 
		exit(1);
	}
	
	int connectionSocket = atoi(argv[1]);
	
	char *text = getfile(connectionSocket, FILE_SIZE - 1); /* text contains:
																plaintext\n
																key\n 		*/
	if(text[0] == 'e')
	{
		char ciphertext[FILE_SIZE] = {0}; /* empty string encrypt() will write ciphertext to */
		
		encrypt(text, ciphertext);
		strcat(ciphertext, "!");
			
		int charsRead = sendall(connectionSocket, ciphertext, strlen(ciphertext)); /* send ciphertext to client */
		if (charsRead < 0)
		{
			perror("Error writing to socket.\n");
			exit(1);
		}
	}
	else
	{
		int charsRead = send(connectionSocket, "x", 1, 0); /* send BAD_CLIENT_MESSAGE to client. */
		if (charsRead < 0)
		{
			perror("Error writing to socket.\n");
			exit(1);
		}
	}

	close(connectionSocket); /* Close socket */

return EXIT_SUCCESS;
}

char* getfile(int connectionSocket, int len)
{
	char buffer[FILE_SIZE] = {0};
	char tmpbuffer[FILE_SIZE] = {0};
	char* file_contents = malloc(sizeof buffer);
	
	int charsRead;
	
	while(strchr(buffer, '!') == NULL)
	{
		if (charsRead = recv(connectionSocket, tmpbuffer, len, 0) == -1 )
		{
			perror("ERROR reading from socket\n");
			exit(1);
		}
		strcat(buffer, tmpbuffer);
		memset(tmpbuffer, '\0', FILE_SIZE);
	}
	strcpy(file_contents, buffer);
	//printf("%s", file_contents);
	memset(buffer, '\0', FILE_SIZE);
	return file_contents;
}

void encrypt(char* text, char* ciphertext)
{
	char *saveptr = NULL;
	char *id = strtok_r(text, "\n", &saveptr); /* "e" */
	char *plaintext = strtok_r(NULL, "\n", &saveptr);
	char *key = strtok_r(NULL, "\n", &saveptr);
	
	if(strlen(key) < strlen(plaintext))
	{
		ciphertext[0] = BAD_KEY;
	}
	
	else
	{
		for(int i = 0; i < strlen(plaintext); i++)
		{	
			char keyval; /* keyval is used to assign each letter to a numerical value
							e.g., 'A' is 0, 'B' is 1....
							' ' (32) is set to 26 */
								
			if(key[i] == 32) 	/* Setting space to 26 */
				keyval = 26;
			else				/* Setting A-Z to 0-25 */
				keyval = key[i] - 65;
				
			char plaintextval; /* same scheme keyval uses*/

			if(plaintext[i] == 32)	/* Setting space to 26 */
				plaintextval = 26;
			else					/* Setting A-Z to 0-25 */
				plaintextval = plaintext[i] - 65;
				
			char cipher = (plaintextval + keyval) % 27;

			cipher += 65;
			if(cipher == 91)
				cipher = 32;
			
			ciphertext[i] = cipher;
			ciphertext[i + 1] = '\n'; /* Appends a newline to the end of the encrypted message.*/
		}
	}
}


