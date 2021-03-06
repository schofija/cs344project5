/*
	decrypt.c
	usage: ./decrypt connectionSocket
	
	Ran by dec_server after a fork()
	This program:	1. gets the text sent from the client (ciphertext & key)
					2. decryptes ciphertext
					3. sends the plaintexttext back to the client
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

#ifndef DEC_VALIDATION /* validates connection is coming from enc_client */
	#define DEC_VALIDATION 'd'
#endif

#ifndef BAD_CLIENT_MESSAGE
	#define BAD_CLIENT_MESSAGE 'x'
#endif

#ifndef BAD_KEY
	#define BAD_KEY 'k'
#endif

int sendall(int, char*, int);
char* getfile(int, int); //socketFD, buffer, len
void decrypt(char*, char*); /* 	usage: text, plaintext, 
								decrypted string written to plaintext */
								
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
	if(text[0] == 'd')
	{
		char plaintext[FILE_SIZE] = {0}; /* empty string encrypt() will write ciphertext to */
		decrypt(text, plaintext);;
				strcat(plaintext, "!");
			
		int charsRead = sendall(connectionSocket, plaintext, strlen(plaintext)); /* send ciphertext to client */
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

/*
Function to receive very long sends
*/
char* getfile(int connectionSocket, int len)
{
	char buffer[FILE_SIZE] = {0};
	char tmpbuffer[FILE_SIZE] = {0};
	char* file_contents = malloc(sizeof buffer);
	
	int charsRead;
	
	while(strchr(buffer, '!') == NULL) /* Loop until "!" (our end-of-file key) is found*/
	{
		if (charsRead = recv(connectionSocket, tmpbuffer, len, 0) == -1 ) /* receive data */
		{
			perror("ERROR reading from socket\n");
			exit(1);
		}
		strcat(buffer, tmpbuffer); /* Add data to our already-received buffer */
		memset(tmpbuffer, '\0', FILE_SIZE); /* reset tmpbuffer*/
	}
	strcpy(file_contents, buffer);
	//printf("%s", file_contents);
	memset(buffer, '\0', FILE_SIZE);
	return file_contents;
}

/* decrypts */
void decrypt(char* text, char* decryptedtext)
{
	char *saveptr = NULL;
	char *id = strtok_r(text, "\n", &saveptr); /* "d" */
	char *plaintext = strtok_r(NULL, "\n", &saveptr); /* ciphertext */
	char *key = strtok_r(NULL, "\n", &saveptr); /* key */
	
	if(strlen(key) < strlen(plaintext))
	{
		decryptedtext[0] = BAD_KEY;
	}
	
	else
	{
		for(int i = 0; i < strlen(plaintext); i++)
		{	
			int keyval;
			if(key[i] == 32) 	/* Setting space to 26 */
				keyval = 26;
			else				/* Setting A-Z to 0-25 */
				keyval = key[i] - 65;
			
			int plaintextval;
			

			plaintextval = plaintext[i] - 65;
			if(plaintextval == -33)
				plaintextval = 26;
				
			int cipher = plaintextval - keyval;
			if(cipher < 0)
				cipher += 27;
			cipher %= 27;
			
			/* Convert back to ascii code */
			//printf("plaintextval: %i (plaintext==%c), keyval: %i, cipher[%i]: %i\n", plaintextval, plaintext[i], keyval, i, cipher);
			if(cipher == 26)
				cipher = 32;
			else
				cipher += 65;
			decryptedtext[i] = cipher;
			decryptedtext[i + 1] = '\n';
		}
	}
}