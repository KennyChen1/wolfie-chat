#include "./include/server.h"
#include "sfwrite.c"

userData * userList;
accountsList *accounts;
loginQueue *front = NULL;
loginQueue *rear = NULL;
int numOnline = 0;
int parentfd; /* parent socket */
int childfd; /* child socket */
int firstu = false;
fd_set comFds;
int selectPipe[2];
bool debug = true;
bool verbose = false;
bool created = true;
FILE *fp;
char receiveBuffer[1024];
char* file = "accounts";
int threadCountNum = 2;

bool fileProvided = false;


pthread_mutex_t userMutex; 
pthread_mutex_t accountMutex;
pthread_mutex_t sfwriteMutex;
pthread_mutex_t Q_lock = PTHREAD_MUTEX_INITIALIZER;
sem_t items_sem;

int main (int argc, char ** argv) {

  if (signal(SIGINT, sig_handler) == SIG_ERR)
    printf("\ncan't catch SIGINT\n");


  handleArguements(argc, argv);


return 0;

}

void printServerHelpMenu(void){
  printf("\nUsage:\n");
  printf("/users\tDumps a list of currently logged on user to stdout\n");
  printf("/help\tPrints out the server help commands\n");
  printf("/shutdown\tDisconnects all users and saves all states and close all sockets\n\n");
}

int handleArguements(int argc, char ** argv){
  if(argc ==  1){
    printf("No arguements passed\n");
    printHelpMenu();
    return EXIT_SUCCESS;
  }
  int opt;
  int numopt = 0;

  while((opt = getopt(argc, argv, "hvt")) != -1) {
    switch(opt) {
      case 'h':
                /* The help menu was selected */
      printHelpMenu();
      numopt++;
        //exit(1);
      break;      
        case 'v':
        verbose = true;
        numopt++;
      break;
        case 't':
        numopt+=2;
        threadCountNum = atoi(argv[numopt]);
        printf("the number is %d\n", threadCountNum);
      break;
      default:
      printHelpMenu();
      exit(EXIT_FAILURE);
      break;
    }
  }

/////////////////////



  if (argc == (4 + numopt))
    fileProvided = true;
  //end of argc == 3


  if (argc == 3 +numopt || fileProvided || verbose){


  int portno; /* port to listen on */
  socklen_t clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  char buffer[1024]; /* message buffer */
  int optval; /* flag value for setsockopt */


    if (fileProvided){
      printf("file provided %s\n", argv[3+numopt]);

      accounts = NULL;


      fp = loadFile(argv[3+numopt]);


      loadFileIntoList(argv[3+numopt], fp);


    } 

    fp = loadFile(file);


    int notdone;
    fd_set readfds;
    int rc;
    pthread_t * tid = malloc(sizeof(pthread_t)*threadCountNum);
    int i;

    portno = atoi(argv[1+numopt]);
    printf("Currently listenting to port: %d\n", portno);
          pthread_mutex_lock(&userMutex);

    parentfd = socket(AF_INET, SOCK_STREAM, 0);
    if (parentfd < 0) 
      perror("ERROR opening socket");
          pthread_mutex_unlock(&userMutex);

    optval = 1;
    setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);

  // bind: associate the parent socket with a port 
    if (bind(parentfd, (struct sockaddr *) &serveraddr, 
      sizeof(serveraddr)) < 0) 
      perror("ERROR on binding");

  //listen: make this socket ready to accept connection requests 
  if (listen(parentfd, 128) < 0) /* allow 5 requests to queue up */ 
    perror("ERROR on listen");


  /* initialize some things for the main loop */
    clientlen = sizeof(clientaddr);
    notdone = 1;

    printf("server> ");
    fflush(stdout);         
    pipe(selectPipe);

   // main loop: wait for connection request or stdin command.
   //If connection request, then echo input line and close connection. 
   //If command, then process command.

    sem_init(&items_sem, 0, 0);

    while (notdone) {
//ACCEPT THREAD
     // select: Has the user typed something to stdin or has a connection request arrived?
    FD_ZERO(&readfds);            //zero out the fd to multiplex std and socket
    FD_SET(parentfd, &readfds);   //add fd to multiplex
    FD_SET(0, &readfds);          //add stdin to multplex
    



      struct userData sendData ; 
      //initilize a struct to pass multiple data into login thread
          // set fd to send fd
     



      //create login threads
      if(created){
      for (i = 0; i < threadCountNum; i++){
        rc = pthread_create(&tid[i], NULL, loginThread, "");//spawn login thread
        if (rc == 0){
          rc = pthread_setname_np(tid[i], "LOGIN");
        }
        else if (rc != 0){
          sfwrite(&sfwriteMutex, stdout, "error");
        }

        }
        created = false;
      } 

    if (select(parentfd+1, &readfds, 0, 0, 0) < 0) {
      perror("ERROR in select");
    }

    /* if the user has entered a command, process it */
    if (FD_ISSET(0, &readfds)) {
      fgets(buffer, 1024, stdin);

      if(!strcmp(buffer, "/help\n")){
        printHelpMenu();


      } else if(!strcmp(buffer, "/shutdown\n")){
        //cleanly disconnect all connected users. close all socks and files and free any heap memory allocated


          shutdownServer(parentfd);



          } else if(!strcmp(buffer, "/users\n")){
            printf("Users online: %d \n", numOnline);
            printf("%s", printAllUsers(0));
          }else if (!strcmp(buffer, "/accts\n")){
            printAccountList();

          } else{
            //printf(".%s.\n", buffer);
            printf("invalid command\n");
          }

          printf("server> ");
          fflush(stdout);

        }    

        if (FD_ISSET(parentfd, &readfds)) {

          pthread_mutex_lock(&userMutex);

          childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);


          sendData. username = argv[2+numopt];  // set username to pass MOTD
          sendData.fd = childfd;  
          sendData.filename = argv[3+numopt]; 
          sendData.fileProvided = fileProvided; 



          pthread_mutex_lock(&Q_lock);
          Enqueue(sendData);
          pthread_mutex_unlock(&Q_lock);

          sem_post(&items_sem);

          FD_SET(childfd, &comFds); //adds client socket to fd_set




    }//end of fd server

} //end of argc == 4


}
return 0;
}

void shutdownServer(int parentfd){
  if (userList != NULL){
    while(userList != NULL){
      send (userList -> fd,"BYE \r\n\r\n", strlen("BYE \r\n\r\n"), 0);
      if (verbose)
        printf("\x1b[1;32mBYE \r\n\r\n\x1b[0m");
      pthread_mutex_lock(&userMutex);
      close(userList -> fd);
      pthread_mutex_unlock(&userMutex);
      removeUser(userList -> username);

      if (userList != NULL){
        printf("next \n");
      }else{
        remove(file);
        close (parentfd);
        exit(0);
      }
    }
  }else{
    remove(file);
    close(parentfd);
    exit(0);
  }
}



void *loginThread(void *vargp){

  char buffer[1024];
  memset(buffer, 0, 1024);

  pthread_detach(pthread_self());



  pthread_t tid;
  int rc;



  while(true){
    
    sem_wait(&items_sem);
   

    userData x = front -> sendData;

    pthread_mutex_lock(&Q_lock);
    Dequeue();
    pthread_mutex_unlock(&Q_lock);

    validateAccountName(x.fd, getUsername(), newOrExisting(x.fd), x.username, x.fileProvided);

    pthread_mutex_unlock(&userMutex);

    pthread_mutex_lock(&userMutex);

    FD_ZERO(&comFds);
    FD_SET(selectPipe[0], &comFds);
    pthread_mutex_unlock(&userMutex);

    struct userData *curNode = userList;
    while(curNode != NULL){
  //printf("curNode: %d\n", curNode->fd);
      FD_SET(curNode->fd, &comFds);
      curNode = curNode -> next;
    }
    write(selectPipe[1], " ", 1);
  ////////////////////////////////////////////////////////////////////////////////////////////

    if(firstu == 0){
      rc = pthread_create(&tid, NULL, communicationThread, (void *)(intptr_t)(0));
      if (rc == 0 ){
        firstu = true;
        rc = pthread_setname_np(tid, "COMMUNICATION");
      }
      else if (rc != 0){
        sfwrite(&sfwriteMutex, stdout, "error");
      }
    }
    

      

  }


  return NULL;
}

void sig_handler(int signo){
  if (signo == SIGINT){
    shutdownServer(parentfd);
     
  } 
}





void *communicationThread(void *vargp){
  char buffer[1024];
  memset(buffer, 0, 1024);
  char bye[1024] = "BYE \r\n\r\n";
  char userOff[1024] = "UOFF ";

  pthread_detach(pthread_self());

  fd_set rdySet;
  while(1){
    if(numOnline == 0){
      firstu = false;
      break;
    }



    memset(buffer, 0, 1024);
    rdySet = comFds;
    select(numOnline+7, &rdySet, 0, 0, 0);
    struct userData *curNode = userList;
    while(curNode != NULL){
      if(FD_ISSET(curNode->fd, &rdySet)){
        read(curNode->fd, buffer, 1024);
        if(verbose)
          printf("\x1b[1;32m%s\x1b[0m", buffer);

        if(!strcmp(buffer, "TIME \r\n\r\n")){
          write(curNode->fd, getTime(curNode->fd), strlen(getTime(curNode->fd)));
          if(verbose)
            printf("\x1b[1;32m%s\x1b[0m", getTime(curNode->fd));

        } else if(!strcmp(buffer, "LISTU \r\n\r\n")){
          write(curNode->fd, printAllUsers(1), strlen(printAllUsers(1)));
          if(verbose)
            printf("\x1b[1;32m%s\x1b[0m", printAllUsers(1));

        } else if (strcmp(buffer, "BYE \r\n\r\n") == 0){


//receive bye from client
//printf("value of buffer should be bye: %s\n", buffer);
//if received bye, send bye


              if (strcmp(buffer, bye )== 0){

                send(curNode -> fd, bye, strlen(bye), 0); //send bye to client

                strcat(userOff, curNode -> username);     //strcat message before removing from list
                strcat(userOff, " \r\n\r\n");


                close(curNode -> fd);                     //close fd and remove from list
                removeUser(curNode -> username);

//user is logged out
//send UOFF <name> \r\n\r\n to all other open clients
                userData *sendUserList = userList;
                while(sendUserList != NULL){
                  send(sendUserList -> fd, userOff, strlen(userOff), 0);
                  if(verbose)
                    printf("\x1b[1;32m%s\x1b[0m",userOff);

                      if (sendUserList -> next != NULL){
                        sendUserList = curNode -> next;
                      }else{
                        break;
                      }

                    }


                  }
                }


                else if(strstr(buffer, "MSG ")){
                  char tempBuffer[strlen(buffer)];
                  strcpy(tempBuffer, buffer);


                  strtok(buffer, " ");

                  int tofd = -1;
                  char name[1024];
                  strcpy(name, strtok(NULL, " "));
                  userData * temp = userList;
                  while(temp != NULL){
                    if(!strcmp(temp -> username, name)){
                      tofd = temp->fd;
                      break;
                    }                      
                    temp = temp -> next;
                  }


                  
                  strtok(NULL, " ");
                 
                  char * sends = strtok(NULL, "");
                  sends[strlen(sends)-6] = 0;
                  printf("sends %s\n", sends);

                  write(curNode->fd, tempBuffer, strlen(tempBuffer));
                  write(tofd, tempBuffer, strlen(tempBuffer));

                }

              }
              curNode = curNode -> next;
            }
          }
          return NULL;
    }




void writeAccount(char * filename, char *username, unsigned char *salt, unsigned char * hash){

	fp = fopen(filename, "a");

	accountsList *list = malloc(sizeof(accountsList));
	list = accounts;





	char *user = list -> username;
	unsigned char * salty = list -> salt;
	unsigned char * hashy = list -> hash;






	fputs(user, fp);
	fputs("::::::::", fp);

	fprintf(fp, "%s::::::::%s", salty, hashy);

	fputs("\n\n\n", fp);


	fclose(fp);


}

FILE* loadFile(char * filename){

	return fopen(filename, "a+");

}

void addAccount(char *username, char *password, unsigned char * salt, unsigned char * hash){

	pthread_mutex_lock(&accountMutex);
	accountsList *temp = malloc(sizeof(accountsList));

	strtok(username, "\r");
	if (accounts == NULL){
	  accounts = temp;
	  accounts -> next = NULL;

	}else{
	  temp -> next = accounts;
	  accounts = temp;
	}
	temp -> username = malloc(strlen(username));
	strcpy(temp -> username, username);

	temp -> password = malloc(strlen(username));
	strcpy(temp -> password, password);

	temp -> salt = malloc(128);

	memcpy(temp -> salt ,salt, 128);

	temp -> hash = malloc(1024);

	memcpy(temp -> hash ,hash, 1024);

	pthread_mutex_unlock(&accountMutex);

//printf("\n\n\n%s   %s\n", temp -> username, salt);

}

void addUser(char * username, int fd, time_t connectTime, char * password){

	userData * temp = malloc(sizeof(userData));

	if(userList == NULL){
	  userList = temp;
	  userList -> next = NULL;
	} else{
	  temp -> next = userList;
	  userList = temp;  //update new head
	}
	temp -> username = malloc(sizeof(char*));

	strcpy(temp -> username, username);
	temp -> fd = fd;
	temp -> connectTime = connectTime;



	numOnline++;
}



int removeUser(char *username){
  userData *temp, *prev;
  temp = userList;

  if (temp!= NULL){
    while (temp != NULL){
      if (strcmp(temp -> username, username) == 0){

        if (temp == userList){
          userList = temp -> next;
          free(temp);
          numOnline--;
          return 1;

        }else{
          prev -> next = temp -> next;
          free (temp);
          return 1;
        }
      }else {
        prev = temp;
        temp = temp -> next;
      }
    }

  }


  return 0;
}


char * getTime(int fd){
  time_t returnTime = -1;
  userData * temp = userList;
  while(temp != NULL){
    if(fd == temp -> fd)
      returnTime = time(NULL) - temp -> connectTime;
    temp = temp -> next;
  }

  char * returnText = malloc(sizeof(returnText));

  sprintf(returnText, "EMIT %ld \r\n\r\n", returnTime);
  return returnText;

}

bool newOrExisting (int childfd){
  //char receiveBuffer[1024];
  char sendVerb[100]= "EIFLOW \r\n\r\n";
  bool newUser = false;
  char *token; //username
  char phrase[100];

  memset(receiveBuffer, 0, sizeof(receiveBuffer));

  //receive 1 WOLFIE
  Recv(childfd, receiveBuffer); 
  if(verbose)
    printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);
    //protocol is initiated


  if (strcmp(receiveBuffer, "WOLFIE \r\n\r\n") == 0){

//send 1 eiflow
    if (send(childfd, sendVerb, strlen(sendVerb), 0) >= 0){
		  if(verbose)
		    printf("\x1b[1;32m%s\x1b[0m\n", sendVerb);

		//if EIFLOW sent successfuly, receive IAMNEW or IAM <username>

		//receive 2 identity
		    memset(receiveBuffer, 0, 1024);
		    Recv(childfd, receiveBuffer);
		    if(verbose)
		      printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);



		    strcpy(phrase, receiveBuffer);



		  	token = strtok(phrase, " "); //get either IAMNEW or IAM


		  if (strcmp(token, "IAMNEW") == 0){

		    newUser = true;
		    return newUser;

		  }
		  else if (strcmp(token, "IAM") == 0){

		    newUser = false;
		    return newUser;
		  }
		  else
		    return newUser;


		}
		}

  return NULL;


}

char* getUsername(){


  char * token;
  
  token = strtok(receiveBuffer, " ");
  token = strtok(NULL, " ");
  
  return token;
}



  //validates account names and sends appropriate response
//validAccount = false means username exists, validAccunt = 1 means username does not exist


bool validateAccountName(int childfd, char * username, bool newUser, char * motd, bool fileProvided){

  char * user = malloc(sizeof(user));
  strcpy(user, username);
  bool validAccount = true;
  bool loggedIn = false;
  char helloNewName[1024] = "HINEW ";
  char helloName[100] = "HI ";
  char auth[100] = "AUTH ";
  //char * bye = "BYE \r\n\r\n";
  char * authorizedPass = "SSAP \r\n\r\n";
  char * validPass = "SSAPWEN \r\n\r\n";
  char * token;

  unsigned char salt[128];
  unsigned char *hash;

  //SHA256(const unsigned char *d, size_t n, unsigned char *md)

  accountsList *temp = accounts;
  userData *usersLoggedIn = userList;


  strcat(helloNewName, username);
  strcat(helloNewName, " \r\n\r\n");

  strcat(helloName, user);
  strcat(helloName, " \r\n\r\n");
  

  if (temp != NULL){

    while(temp != NULL){                
      //remove \n and \r\n\r\n
      strtok(temp -> username, "\n");
      strtok(user, "\r");

     
      if (strcmp(user, temp -> username) == 0){
         
        validAccount = false;
        break;

      }else{

        validAccount = true;
      }

      temp  = temp -> next;

    }

    //check to see if user is logged in
    while(usersLoggedIn != NULL){

      strtok(usersLoggedIn -> username, "\n");
      strtok(user, "\r");

      if (strcmp(user, usersLoggedIn -> username) == 0){

        loggedIn = true;
        break;
      }
      else{
        loggedIn = false;
      }

      usersLoggedIn = usersLoggedIn -> next;
    }

      //if name is not taken, send HINEW <username>

    if (validAccount && newUser){

      //send 2 HINEW
    	send(childfd, helloNewName, strlen(helloNewName), 0);
    	if(verbose)
        	printf("\x1b[1;32m%s\x1b[0m\n", helloNewName);


      //receive NEWPASS
    	Recv(childfd, receiveBuffer);
        if(verbose)
        	printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);

      	token = strtok(receiveBuffer, " "); //token = NEWPASS
      	token = strtok(NULL, " "); //token = password

      //if password meets criteria, send SSAPWEN

      	if (validatePassword(token)){
	        //send 3 SSAPWEN
	        send(childfd, validPass, strlen(validPass), 0);
	        memset(salt, 0, 128);
	        RAND_bytes(salt, 128);

	        hash = hashPassword(token, salt);
	  		//login
	        addAccount(user, token, salt, hash);
	        addUser(user, childfd, time(NULL), token);

	        writeAccount(file, user, salt, hash);       

	        //send 4 HI <name>
	        send(childfd, helloName, strlen(helloName), 0);

	        
	        memset(receiveBuffer, 0, 1024);
	        sprintf(receiveBuffer, "MOTD %s \r\n\r\n", motd);
        	if(verbose)
          		printf("\x1b[1;32m%s\n\x1b[0m", receiveBuffer);
            send(childfd, receiveBuffer, strlen(receiveBuffer), 0);
          }else{
            sendError(childfd, 2, "");
          }


          return validAccount;


      	//if name is taken send ERR 00 and bye, terminate connection
        }else if (validAccount == false && newUser){
      	//send 2 ERROR send 3 bye
          sendError(childfd, 0, user);
          return validAccount;

    	//if existing user and account exists, send AUTH <name> 
        }else if (validAccount == false && (newUser == false) && loggedIn == false){

	      strcat(auth, user);
	      strcat(auth, " \r\n\r\n");
     	 //send 2 AUTH <name>

          send(childfd, auth, strlen(auth), 0);
          if(verbose)
            printf("\x1b[1;32m%s\x1b[0m\n", auth);

      	//receive 3, password
          Recv(childfd, receiveBuffer);
          if(verbose)
             printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);

      //strtok to get password
          token = strtok(receiveBuffer, " ");
          token = strtok(NULL, " ");

          strtok(token, "\r");


           //check to see if password is correct
          if (checkExistingPassword(user, token)){//

  			//if password is correct, send SSAP
  			//send 3 SSAP
            send(childfd, authorizedPass, strlen(authorizedPass), 0);
            if(verbose)
              printf("\x1b[1;32m%s\x1b[0m\n", authorizedPass);

  			//send 4 HI

          	send(childfd, helloName, strlen(helloName), 0);
          	if(verbose)
            	printf("\x1b[1;32m%s\x1b[0m\n", helloName);


            addUser(user, childfd, time(NULL), token);

			//send message of the day
            memset(receiveBuffer, 0, 1024);
            sprintf(receiveBuffer, "MOTD %s \r\n\r\n", motd);

            if(verbose)
              printf("\x1b[1;32m%s\n\x1b[0m", receiveBuffer);

            write(childfd, receiveBuffer, strlen(receiveBuffer));

            }else {

            
			//if password is wrong, send error2
            sendError(childfd, 2, "");

            }


              return validAccount;
        }

    	//if existing user but account name does not exist
    	//send error 1 and bye
        else if ((validAccount == true) && (newUser == false)) {
      //send 2 error send 3 bye
	      sendError(childfd, 1, "");

	      return validAccount;
        }

    
    //else if existing user but username is already logged in
        else if ((validAccount == false) && (loggedIn == true)){
      //send 2 error send 3 bye
        	sendError(childfd, 0, "");

            return validAccount;




        }
   }
  else{ //if list is null, username is valid
    if (newUser){
      //send 2 HINEW
      send(childfd, helloNewName, strlen(helloNewName), 0);
      if(verbose)
        printf("\x1b[1;32m%s\x1b[0m", helloNewName);
      //receive NEWPASS

      Recv(childfd, receiveBuffer);

      if(verbose)
        printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);

      token = strtok(receiveBuffer, " "); //token = NEWPASS
      token = strtok(NULL, " "); //token = password

      strtok(token, "\r");
      //if password meets criteria, send SSAPWEN

      if (validatePassword(token)){
      //send 3 SSAPWEN
      	send(childfd, validPass, strlen(validPass), 0);

      	if(verbose)
      		printf("\x1b[1;32m%s\x1b[0m\n", validPass);

      	memset(salt, 0, 128);


      	RAND_bytes (salt, 128);

     
      	hash = hashPassword(token, salt);



      addAccount(user, token, salt, hash);
      addUser(user, childfd, time(NULL), token);

      writeAccount(file, user, salt, hash);
      //send 4 HI name


      send(childfd, helloName, strlen(helloName), 0);
      if(verbose)
      	printf("\x1b[1;32m%s\x1b[0m\n", helloName);


      //send MOTD

      memset(receiveBuffer, 0, 1024);
      sprintf(receiveBuffer, "MOTD %s \r\n\r\n", motd);
      if(verbose)
      	printf("\x1b[1;32m%s\n\x1b[0m", receiveBuffer);
      send(childfd, receiveBuffer, strlen(receiveBuffer), 0);

      validAccount = true;
      return validAccount;
      }else {

      sendError(childfd, 2, "");
      }
    } // end of null list, create new user


   else{

    sendError(childfd, 1, "");

  
  }
}

	return NULL;
}





    //if list is null, username will be valid. send HINEW, return true


//strcmp is returning 3 when password "hello" and temp -> password = "hello"
//need to remove a newline
bool checkExistingPassword (char * user, char* password){
  accountsList *temp = accounts;
  bool correctPassword = false;
  unsigned char * hash;

  //hash = hashPassword(password, temp -> salt);

  
  while(temp != NULL){
    

    
    if (strcmp (user, temp -> username) == 0){


      hash = hashPassword(password, temp -> salt);

      if  (strcmp((char *)hash, (char*)temp -> hash) == 0)

        correctPassword = true;
      break;

      
    }else{
      correctPassword = false;
    }

    temp = temp -> next;
  }

  return correctPassword;
}


void loadFileIntoList (char * filename, FILE* fp){

  //char * pass;
  unsigned char * hashed;
  unsigned char * salt;
  char * username;
  char line[3000];
  


  //fp = fopen(filename, "r+");

  if (fp == NULL){
    printf("file could not be opened");
  }



  while(fgets(line, sizeof(line), fp)){

    printf("\n\nline: %s\n", line);
    
    
    username = strtok(line,  "::::::::");

    printf("\n\nusername: %s\n", username);
    
    salt = (unsigned char *)strtok(NULL, "::::::::");


    printf("\n\nsalt: %s\n", salt);

    hashed = (unsigned char *)strtok(NULL, "::::::::");

    

    printf("\n\nhash: %s\n", hashed);


    
    addAccount(username, "", salt, hashed);



  }

  fclose(fp);


}

void sendError (int childfd, int errorNo, char * user){
  char error[1024] = "ERR ";
  char bye[15] = "BYE \r\n\r\n";
    //char receiveBuffer[1024];

  if (errorNo == 0){
    if(strcmp(user, "") == 0){
      strcat(error, "00 USER NAME TAKEN");
      strcat(error, " \r\n\r\n");

    } else{
      strcat(error, "00 USER NAME TAKEN ");
      strcat(error, user);
      strcat(error, " \r\n\r\n");
    }
    


      //send error message, send bye, receive bye, terminate connection
    send(childfd, error, strlen(error), 0);
    if(verbose)
      printf("\x1b[1;32m%s\x1b[0m\n", error);

      send(childfd, bye, strlen(bye), 0);
      if(verbose)     
      printf("\x1b[1;32m%s\x1b[0m\n", bye);

      printf("terminating connection");
      close(childfd);

      }

      else if (errorNo == 1){
	    strcat(error, "01 USER NOT AVAILABLE \r\n\r\n");
	    send(childfd, error, strlen(error), 0);
      	if(verbose) 
      		printf("\x1b[1;32m%s\x1b[0m\n", error);

      	send(childfd, bye, strlen(bye), 0);
      	if(verbose)
      		printf("\x1b[1;32m%s\x1b[0m\n", bye);

      	close(childfd);
      }

      else if (errorNo == 2){

      	strcat(error, "02 BAD PASSWORD \r\n\r\n");
      	send(childfd, error, strlen(error), 0);
      	if(verbose)
      		printf("\x1b[1;32m%s\x1b[0m\n", error);
      	send(childfd, bye, strlen(bye), 0);
      		if(verbose)
      	printf("\x1b[1;32m%s\x1b[0m\n", bye);
      	close(childfd);
      }


  }

bool validatePassword (char * password){
//at least 5 characters 
//at least 1 uppercase character [65-90]
//at least 1 symbol [33-47 & 58-64 & 91-96 & 123-126]
//at least 1 number [48-57]

	int length = strlen(password);
	int lengthFlag, uppercaseFlag, symbolFlag, numFlag, i;

    if (length < 5){
      lengthFlag = 0;
      printf("password too short");
      return false;
    }
    else{

    	lengthFlag = 1;

		//number check
     	for (i = 0; i < length; i++){
        	if (((password[i] >= 48) && (password[i] <= 57)) || password[i] == '\n'){
		//contains a number
          	numFlag = 1;
          	break;

        } else{
          	numFlag = 0;
        }

      }

		//uppercase check
      	for (i = 0; i < length; i++){
        	if (((password[i] >= 65) && (password[i] <= 90)) || password[i] == '\n'){
			//contains an uppercase
	        	uppercaseFlag = 1;
	          	break;
        	}else{
          		uppercaseFlag = 0;

        }
      }

		//symbol check

      for (i = 0; i< length; i++){
        if (((password[i] >= 33) && (password[i] <= 47)) || ((password[i] >= 58) && (password[i] <= 64)) 
          || ((password[i] >= 91) && (password[i] <= 96)) || ((password[i] >= 123) && (password[i] <= 126)) || password[i] == '\n' ){
		//contains a symbol
         	symbolFlag = 1;
        	break;
      }else{
        symbolFlag = 0;

      }
    }

  }

  if (lengthFlag == 0 || numFlag == 0 || uppercaseFlag == 0 || symbolFlag == 0){
    printf("Invalid password\n");
    return false;
  }else{
    printf("Password verified\n");
    return true;
  }


}

unsigned char * hashPassword(char * password, unsigned char * salt){


	unsigned char *md = malloc(SHA256_DIGEST_LENGTH);
	SHA256_CTX context;
	char * space = malloc(128 + strlen(password));

	memset(space, 0, 128 + strlen(password));



	//copy salt to space
	memcpy(space, salt, 128);


	//append password to salt
	memcpy(space + 128, password, strlen(password));


	SHA256_Init(&context);
	SHA256_Update(&context, space, 128 + strlen(password));
	SHA256_Final(md, &context);



	return md;
}



void printHelpMenu(){
    printf("\nUsage: \t./server [-hv] [-t THREAD_COUNT] PORT_NUMBER MOTD [ACCOUNTS_FILE]\n");
    printf("-h \t\tDisplays this help menu and returns EXIT_SUCCESS.\n");
    printf("-v \t\tVerbose print all incoming and outgoing protocol verbs and content.\n");
    printf("-t THREAD_COUNT\t\tThe number of threads user for the login queue.\n");
    printf("PORT_NUMBER\tPort number to listen on.\n");
    printf("MOTD\t\tMessage to display to the client when they connect\n");
    printf("ACCOUNTS_FILE\t\tFile containing username and password data to be loaded upon execution\n\n");
  }

  char * printAllUsers(int boo){
	struct userData *curNode = userList;

	char listUsers[2048];
	if(boo == 0)
		strcpy(listUsers, "USERS\n");
	else
		strcpy(listUsers, "UTSIL ");
	while(curNode != NULL){ 
		strcat(listUsers, curNode->username);
		strcat(listUsers, " \r\n");
		curNode = curNode -> next;
	}
	if(boo != 0)
		strcat(listUsers, " \r\n\r\n");

		char *listU = malloc(102114);
		strcpy(listU, listUsers);
		return listU;
  }

void printAccountList(){
  printf("ACCOUNTS\n");
  accountsList *curNode = accounts;
  while (curNode != NULL){
    printf("%s\t%s\t%s\n", curNode -> username, curNode -> salt, curNode -> hash);
    curNode = curNode -> next;
  }
}

void Recv(int fd, char * buffer){
  int i = 0; 

  while(1){
    recv(fd, buffer, 1, 0);
    buffer++;

    i++;
    //printf("%d\t", buffer[-1]);
    if(strstr(buffer-i, " \r\n\r\n")){

      break;
    }

  } 
      buffer = buffer - i;
}

void Enqueue(userData sendData){
  loginQueue *temp = (loginQueue*)malloc(sizeof(loginQueue));

  temp -> sendData = sendData;
  temp -> next = NULL;
  if (front == NULL && rear == NULL){
    front = rear = temp;
    return;
  }
  rear -> next = temp;
  rear = temp;
}

void Dequeue(){
  loginQueue *temp = front;
  if (front == NULL){
    sfwrite(&sfwriteMutex, stdout, "Queue is empty");
    return;
  }
  if (front == rear){
    front = rear = NULL;
  }else{
    front = front ->next;
  }
  free(temp);
}