/* - - - - How to Use? - - - -
 * The command line usage:
 * 	- Server mode: programname [port_number]
 * 	- Clinet mode: programname [port number] [ip address]
 * - - - - - - - - - - - - - - - - - */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>

#define error(a,b) fprintf(stderr, a, b)  // Error handling
#define BUFFSIZE 1024			// The data buffer size
#define MAXCOM 16				// The command buffer size
#define MAXTIPS 10				// The number of maximum tips


/* - - - Coloring Header - - - */
#define ANSI_COLOR_RED		"\x1b[31m"
#define ANSI_COLOR_GREEN		"\x1b[32m"
#define ANSI_COLOR_YELLOW	"\x1b[33m"
#define ANSI_COLOR_BLUE		"\x1b[34m"
#define ANSI_COLOR_MAGENTA	"\x1b[35m"
#define ANSI_COLOR_CYAN 		"\x1b[36m"
#define ANSI_COLOR_RESET		"\x1b[0m"
#define ANSI_BOLD   			"\x1b[1m"
/* - - - - - - - - - - - - - - - - - - */

void Exit_if_Error(int IsNotPositive, int ErrorCode){
	if(IsNotPositive < 0){
		//For error logging
		switch(ErrorCode){
			case 10: fprintf(stderr, "Socket creation error.\n"); break;
			case 11: fprintf(stderr, "Socket bind error.\n"); break;
			case 12: fprintf(stderr, "Socket listen error.\n"); break;
			case 40: fprintf(stderr, "Client connection error.\n"); break;
			case 41: fprintf(stderr, "Client acknowledgement error.\n"); break;
			case 42: fprintf(stderr, "Client message content error.\n"); break;
			case 43: fprintf(stderr, "Recieve from client error.\n"); break;
			case 44: fprintf(stderr, "Send to client errorr.\n"); break;
			case 50: fprintf(stderr, "Server connection error.\n"); break;
			case 51: fprintf(stderr, "Server acknowledgement error.\n"); break;
			case 52: fprintf(stderr, "Server message content error.\n"); break;
			case 53: fprintf(stderr, "Recieve from server error.\n"); break;
			case 54: fprintf(stderr, "Send to server errorr.\n"); break;
			default: ;
		}
		exit(ErrorCode);
	}else{
		//For state logging
		switch(ErrorCode){
			case 10: fprintf(stderr, "Socket created.\n"); break;
			case 11: fprintf(stderr, "Socket binded.\n"); break;
			case 12: fprintf(stderr, "Socket listening.\n"); break;
			case 40: fprintf(stderr, "Client connectioned.\n"); break;
			/*
			case 41: fprintf(stderr, "Client acknowledged.\n"); break;
			case 42: fprintf(stderr, "Good client message.\n"); break;
			case 43: fprintf(stderr, "Recieved from client.\n"); break;
			case 44: fprintf(stderr, "Sent to client.\n"); break;
			*/
			case 50: fprintf(stderr, "Connected to server.\n"); break;
			/*
			case 51: fprintf(stderr, "Server acknowledged.\n"); break;
			case 52: fprintf(stderr, "Good server message.\n"); break;
			case 53: fprintf(stderr, "Recieved from server.\n"); break;
			case 54: fprintf(stderr, "Sent to server.\n"); break;
			*/
			default: ;
		}
	}
}

void Acknowledge(int willSend, int isServer, int fp, char* buffer, const char* OKmsg){
	if(willSend){
		/* Sending acknowledgement */
		Exit_if_Error(write(fp, OKmsg, strlen(OKmsg)+1), isServer ? 43 : 53);
	}else{
		/* Waiting for acknowledgement */
		Exit_if_Error(read( fp, buffer, strlen(OKmsg)+1), isServer ? 43 : 53);
		if(strcmp(buffer, OKmsg))		Exit_if_Error(-1, isServer ? 41 : 51);
	}
}

/* Declarations */
int server_socket;				// socket endpt of server
int client_socket;				// socket endpt of client
struct sockaddr_in server_addr;	// socket addr of server
struct sockaddr_in client_addr;		// socket addr of client
int server_size = sizeof server_addr;// length of the socket addr. server
int client_size = sizeof client_addr;	// length of the socket addr. client
int rcvsize;					// received bytes
int trnmsize;					// transmitted bytes
char one = 1;                        		// Just 1
char buf[BUFFSIZE+1];			// data buf
char command[MAXCOM+1];		// command buffer
int estimated = 0;				// The last estimation of the user
int factual = 0; 					// The random generated number by the server
int tips = 0;					// number of tips
int end = 0;					// flag to end the game
int shtdwn = 0;					// flag to end the session
char OKmsg[] = "Received.";		//The acknowledgement message.		

int main(int argc, char *argv[] ){
	
	/* ----------------- */
	/* Wrong usage */
	/* ---------------- */
	if(argc <= 1){
		printf("%sError: Wrong parameter usage!\n\t%s%sHow to use?%s\n", ANSI_COLOR_RED, ANSI_COLOR_GREEN, ANSI_BOLD, ANSI_COLOR_RESET);
		printf("Command line usage:\n    - Server mode: programname [port_number]\n    - Clinet mode: programname [port number] [ip address]\n");
	}
	else{
		// Random generator initialization
		srand(time(NULL)); 
		
		/* Server Initialization */
		server_addr.sin_family	=	AF_INET;		// Family type
		server_addr.sin_port		=	htons(atoi(argv[1]));	// Port number
		
		/* Variable Initialization */
		strncpy(buf, "\0", BUFFSIZE);
		strncpy(command, "\0", MAXCOM);
		
		/* ---------- */
		/* CLIENT */
		/* ---------- */
		if(argc > 2){
			/* Run type feedback */
			printf("Running as client.\n");
			
			/* Connect to the selected server. It's address is the first command line argument */
			server_addr.sin_addr.s_addr = inet_addr(argv[2]);	// Server address
			
			/* Creating socket */
			server_socket = socket(AF_INET, SOCK_STREAM, 0);
			Exit_if_Error(server_socket, 10);
			
			/* Setting socket options */
			setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof one);
			setsockopt(server_socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&one, sizeof one);
			
			/* Connecting to the server */
			Exit_if_Error(connect(server_socket, (struct sockaddr*) &server_addr, server_size), 50);
			
			/* Create a game session with the server*/
			while(!shtdwn){
				/* Start a New Game and give Feedback */
				printf("\n%s - - - - - - - %s\nStarting a new game...%s\n", ANSI_COLOR_YELLOW, ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
				end = 0;
				
				/* Run the game */
				while(!end){
					// Read the server's message.
					Exit_if_Error(read(server_socket, buf, BUFFSIZE), 53);
					
					//Determine the game's state.
					if (!strcmp(buf, "Smaller")){
						//If the tip is too big.
						printf("Your tip is too big. Make an another tip.\n");
					}else if(!strcmp(buf, "Bigger")){
						//If the tip is too small.
						printf("Your tip is too small. Make an another tip.\n");
					}else if(!strcmp(buf, "Correct")){
						//If the tip is correct.
						printf("Your tip is correct.\nCongratulations you won the game!\n");
						end = 1;
					}else if(!strcmp(buf, "Start")){
						//Starting the game.
						printf("The game is started. You can tip now.\n");
					}else if(!strcmp(buf, "Over")){
						//If you run out of tips.
						printf("You don't have any remaining tips.\nYou lose the game!\n");
						end = 1;
					}else{
						//Not expected state. Something is wrong if the softvare manages to reach it.
						printf("Well, your sofware reached an unexpected state.\n");
						printf("Congratulations! Achievement unlocked: How did we get there?\n");
						printf("But I think you should be able to tipp, if the server is functional.\n    Your tip:    ");
					}
					
					client_endgame:
					
					// Tip if the game not ended
					if(!end){
						// Get a coorect tip from the user and send it to the server.
						char* endptr = NULL;
						do{
							printf("    Your tip:    %s%s", ANSI_COLOR_RED, ANSI_BOLD);
							scanf("%s", buf);
							printf("%s", ANSI_COLOR_RESET);
							estimated = strtol (buf, &endptr, 10);
						}while( (buf == endptr) || (estimated < 1) || (estimated > 100) );
					}
					// Send command, if it ended.
					else{
						/* Ask about new game */
						printf("The game ended. You can use the following commands:\n");
						printf("%s%s - [new]:%s %s\n%s%s - [end]:%s %s\n%s    %s%s",
						ANSI_BOLD, ANSI_COLOR_YELLOW, ANSI_COLOR_RESET,
						"Start a new game.",
						ANSI_BOLD, ANSI_COLOR_YELLOW, ANSI_COLOR_RESET,
						"Finish the current game session and end the connection with the server.",
						"Do you want to start a new game?",
						ANSI_BOLD, ANSI_COLOR_RED
						);
						scanf("%s", buf);
						printf("%s", ANSI_COLOR_RESET);
						
						/* Save the command */
						strncpy( command, buf, MAXCOM);
					}
					
					Exit_if_Error(write(server_socket, buf, BUFFSIZE), 44);
					Acknowledge(0, 0, server_socket, buf, OKmsg);		// Acknowledgement proccess
				
				}
				
				/* If we end the session, then it's sleepy-sleppy time! */
				if( !strcmp( command, "end") ){
					shtdwn = 1;
				}
				else if(!strcmp( command, "new")){
					end = 0;
				}else{
					printf("%sWrong command!\n%s", ANSI_COLOR_RED, ANSI_COLOR_RESET);
					goto client_endgame;
				}
			}
		}
		
		/* ------------ */
		/* SERVER */
		/* ------------ */
		else{
			/* Run type feedback */
			printf("Running as server.\n");
			
			/* This is the server */
			server_addr.sin_addr.s_addr = INADDR_ANY;	// Server address
			
			/* Creating socket */
			server_socket = socket(AF_INET, SOCK_STREAM, 0);
			Exit_if_Error(server_socket, 10 );
			
			/* Setting socket options */
			setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof one);
			setsockopt(server_socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&one, sizeof one);
			
			/* Binding socket */
			Exit_if_Error( bind(server_socket, (struct sockaddr*) &server_addr, server_size), 11 );
			
			/* Listening */
			Exit_if_Error( listen(server_socket, 10), 12 );
			
			/* Accepting connection request */
			client_socket = accept(server_socket, (struct sockaddr*) &client_addr, &client_size);
			Exit_if_Error( client_socket, 40 );
			
			/* Create a game session with the client*/
			while(!shtdwn){
				/* Start a New Game and give Feedback */
				printf("\n%s - - - - - - - %s\nStarting a new game...%s\n", ANSI_COLOR_YELLOW, ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
				tips = 0;
				end = 0;
				
				/* Create a random number between 1 and 100 */
				factual = (rand() % 100) + 1;
				printf("The number is: %i\n", factual);
				
				/* Run the game */
				while(!end){
					// Determine the stage of the game
					if(tips < 1){
						sprintf(buf, "Start");
					}else if(tips < MAXTIPS){
						// Read in the estimated value from the client.
						if(estimated - factual){
							sprintf(buf, (estimated < factual) ? "Bigger" : "Smaller");
						}else{
							sprintf(buf, "Correct");
							printf("The client is victorious!\n");
							end = 1;
						}
					}else{
						sprintf(buf, "Over");
						printf("The client suffered a horrific defeat!\n");
						end = 1;
					}
					
					// Send the right data to the client
					Exit_if_Error(write(client_socket, buf, BUFFSIZE), 44);
					
					server_endgame:
					
					// Recieve the client's message, if the game not ended yet.
					Exit_if_Error(read(client_socket, buf, BUFFSIZE), 43);
					
					//The message is the tip, if the game not ended yet.
					if(!end){
						estimated = atoi(buf);
						// Increment the number of readed tips
						tips++;
					}
					//The message is the command, if it's the end of the game.
					else{
						/* Save the command */
						strncpy(command, buf, MAXCOM);
						printf("Command: %s\n", command);
					}
					
					// Acknowledgement proccess
					Acknowledge(1, 1, client_socket, buf, OKmsg);
				}
				
				/* If we end the session, then it's sleepy-sleppy time! */
				if( !strcmp( command, "end") ){
					shtdwn = 1;
				}
				else if(!strcmp( command, "new")){
					end = 0;
				}else{
					goto server_endgame;
				}
			}
		}
		
		/* Close the sockets */
		close(server_socket);
		close(client_socket);
	}
}