#include <stdio.h>
#include <string.h>
#include <stdbool.h>  
#include <unistd.h> 
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>

// Assume no input line will be longer than 1024 bytes




int main (int argc, char ** argv, char **envp) {
	
	if(argc == 2){
		fd_set chatfds;
		char buffer[1024];
		char connected[1024];
		memset(connected, 0, 1024);
		read(atoi(argv[1]), connected ,1024);		
		strcat(connected, " \r\n\r\n");
		printf("to: %s\n", connected);

		while(1){

			FD_ZERO(&chatfds);
			FD_SET(1, &chatfds);
			FD_SET(atoi(argv[1]), &chatfds);
			select(atoi(argv[1]) +1, &chatfds, 0, 0, 0);
			if(FD_ISSET(1, &chatfds)){
				memset(buffer, 0, 1024);
				fgets(buffer, 1024, stdin);

				
				strcpy(&buffer[strlen(buffer)-1], " \r\n\r\n");
				
					

				
				write(atoi(argv[1]), buffer, strlen(buffer));

				write(atoi(argv[1]), connected, strlen(connected));

				//printf("You typed: %s\n", buffer);
			} else if(FD_ISSET(atoi(argv[1]), &chatfds)){
				memset(buffer, 0, 1024);
				read(atoi(argv[1]), buffer ,1024);
				if(!strcmp(buffer, "haha xd\n")){
					printf("Press Any Key to exit\n");  
					getchar();
					exit(0);
				} else if(!strcmp(buffer, "/close")){
					exit(0);
				}
				printf("%s", buffer);
			}

		}

	} else {
		printf("ERROR USAGE: ./chat <fd>\n");
	}


}