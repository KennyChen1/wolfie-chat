#include "./include/client.h"
#include "sfwrite.c"


userConnected *connections;
bool verbose = false;
bool create = false;
int sockfd =-1;
char receiveBuffer[1024];
char from[1024];
char to[1024];
FILE *f;
bool justclosed = false;
bool fprovided = false;

  int numopt = 0;
  int main (int argc, char ** argv) {


  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (signal(SIGINT, sig_handler) == SIG_ERR)
    printf("can't catch SIGINT\n");
  if (signal(SIGCHLD, sig_handler) == SIG_ERR)
    printf("can't catch SIGCHLD");
  handleArguements(argc, argv);

return 0;
}

void printClientHelpMenu(void){
  printf("\nUsage:\n");
  printf("/time\tPrints out the duration of time user connected\n");
  printf("/help\tPrints out the client help commands\n");
  printf("/logout\tDisconnects user from server\n");
  printf("/listu\tLists all users connected\n\n");

}
//send wolfie, receive EIFLOW, send IAMNEW <name>


int handleArguements(int argc, char ** argv){
  if(argc == 1){
    printf("No arguements passed\n");
    printHelpMenu();
    exit(EXIT_FAILURE);
  }

  int opt;
  while((opt = getopt(argc, argv, "hvca")) != -1) {
    switch(opt) {
      case 'h':
      printHelpMenu();
      numopt++;
      break;
      case 'v':
      verbose = true;
      numopt++;
      break;
      case 'c':
      create = true;
      numopt++;
      
      break;
      case 'a':
      numopt+=2;
      fprovided = true;
      break;
      default:
                /* A bad option was provided. */
      printHelpMenu();
      exit(EXIT_FAILURE);
      break;
    }
  }



  /////////////////////////////////////////////////////////////////


  if(argc == 4+numopt || create){

if(fprovided){
  printf("%s\n", argv[numopt]);
  f = fopen(argv[numopt], "a+");

} else{
   f = fopen("audit.log", "a+");

  if(f > 0)
    printf("file created\n");
}

 

    int portno;
    struct sockaddr_in serv_addr;

    //struct hostent *server;

    unsigned long inaddr;




    char buffer[1024];//, serverMessage[1024];

      portno = atoi(argv[3+numopt]);
      inaddr = inet_addr(argv[2+numopt]);
    


    bzero((char *) &serv_addr, sizeof(serv_addr));    

    serv_addr.sin_family = AF_INET;   
    serv_addr.sin_port = htons(portno);
    
    serv_addr.sin_addr.s_addr = inaddr;
    


    //serv_addr.sin_addr.s_addr = INADDR_ANY;



    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
      puts("ERROR connecting");

    printf("connection established\n");


    if (create == false){      
      existingUserProtocol(sockfd, argv[1+numopt], argv);
    }
    else{      
      newUserProtocol(sockfd, argv[1+numopt], argv);
    }

    
    memset(receiveBuffer, 0 ,1024);


    char *logout = "/logout\n";
    //char *bye = "BYE \r\n\r\n";
    char serverMessage[1024];

    fd_set readfds; //create fd set
    printf("Enter message : ");
    fflush(0);
    while(1) {

      FD_ZERO(&readfds);
      FD_SET(1, &readfds);
      FD_SET(sockfd, &readfds);

      userConnected * temp = connections;
      while(temp != NULL){
              FD_SET(temp -> pair, &readfds);
              temp = temp -> next;
      }



      select(200 +1, &readfds, 0, 0, 0);


      if(FD_ISSET(1, &readfds)){

        memset(buffer, 0, 1024);
        if(justclosed){
          justclosed = false;
          continue;
        }
        fgets(buffer, 1024, stdin);


    
        if (strcmp(buffer, "/time\n") == 0){


          send(sockfd , "TIME \r\n\r\n" , strlen("TIME \r\n\r\n") , 0);
          
          if(verbose)
            printf("\x1b[1;32mTIME \r\n\r\n\x1b[0m");
        
            memset(serverMessage, 0, 1024);
            //printf("asdfdsf\n");
            Recv(sockfd , serverMessage);  //recieves EMIT <time> \r\n\r\n

            //if(verbose){
            printf("\x1b[1;32m%s\x1b[0m", serverMessage);
            //}
              if(validVerb("EMIT ", serverMessage)){

                writeToAuditLog(f, argv[1+numopt], "CMD", "/time", "succes", "client\n");

                strtok(serverMessage, " ");

                long int totalSecs = atoi(strtok(NULL, " "));

                long int hours = totalSecs/3600;
                long int minutes = (totalSecs%3600)/60;
                long int seconds = ((totalSecs%3600)%60);

                printf("connected for %ld hour(s), %ld minute(s), and %ld second(s)\n", hours, minutes, seconds);

              } else{
                printf("bad verb\n");
                exit(0);
              }

          //send BYE, receive BYE from server.
          /////////////////////////////////////////////////////////////////////////
            }else if(!strcmp(buffer, "/listu\n")){
              send(sockfd , "LISTU \r\n\r\n" , strlen( "LISTU \r\n\r\n") , 0);   //list LISTU verb
            if(verbose)
              printf("\x1b[1;32m%s\x1b[0m", "LISTU \r\n\r\n");
              memset(serverMessage, 0, 1024);

              Recv(sockfd , serverMessage);
              if(validVerb("UTSIL ", serverMessage)){
                writeToAuditLog(f, argv[1+numopt], "CMD", "/listu", "success", "client\n");

              if(verbose)
                printf("\x1b[1;32m%s\x1b[0m", serverMessage);
              printf("%s\n", serverMessage + 6);

            }     
          }
          else if(strstr(buffer, "/chat ") == buffer){
              char msgVerb[2048];
              strtok(buffer, " ");
              char to[1024];
              strcpy(to, strtok(NULL, " "));
              char msg[1024];
              strcpy(msg, strtok(NULL, ""));
              sprintf(msgVerb, "MSG %s %s %s \r\n\r\n", to, argv[1+numopt], msg);
              
              send(sockfd , msgVerb , strlen(msgVerb) , 0);

               writeToAuditLog(f, argv[1+numopt], "CMD", "/chat", "success", "client\n");

              if(verbose)
                printf("\x1b[1;32m%s\x1b[0m", msgVerb);
          } else if(!strcmp(buffer, "/help\n")){
              writeToAuditLog(f, argv[1+numopt], "CMD", "/help", "succes", "client\n");
              printClientHelpMenu();    
          }
          else if (strcmp(buffer, logout) == 0){
                  writeToAuditLog(f, argv[1+numopt], "LOGOUT", "intentional\n", "", "");

            /*
            send(sockfd, bye, strlen(bye), 0);
            if(verbose)
            printf("\x1b[1;32m%s\x1b[0m", bye);
            memset(receiveBuffer, 0, 1024);
          Recv(sockfd, receiveBuffer);
          if(verbose)
          printf("\x1b[1;32m%s\x1b[0m", receiveBuffer);

//if receives BYE from server
        if (strcmp(receiveBuffer, bye) == 0){

        printf("logging out\n");
        */
        logoutClient(verbose);
        break;
        

      } else if(strcmp(buffer, "\n")){

        bool notspace = false;

        for(int i = 0; i < strlen(buffer); i++){
          if (!isspace(buffer[i]))
            notspace = true;
            break;
        }

        if(notspace){
          buffer[strlen(buffer)-1] = 0;
          if(strstr(buffer, "/") == buffer)
            writeToAuditLog(f, argv[1+numopt], "CMD", buffer, "success", "client\n");
          else
            writeToAuditLog(f, argv[1+numopt], "CMD", buffer, "failure", "client\n");
          }
        }

        printf("Enter message : ");
        fflush(0);
        continue;
  }//end of fd_isset for stdin
  else if(FD_ISSET(sockfd, &readfds)){
    
    memset(receiveBuffer, 0, 1024);
    Recv(sockfd, receiveBuffer);
        if(verbose)
        printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);
      if(validVerb("BYE ", receiveBuffer)){

        if(verbose)
        printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);
        printf("shutting down server\n");
        close(sockfd);
        exit(0);

      } 

      else if(validVerb("UOFF ", receiveBuffer)){
        strtok(receiveBuffer, " ");
        char name[1024];
        memset(name, 0, 1024);
        strcpy(name, strtok(NULL, ""));
        name[strlen(name)-5] = 0;
        
        if(getFDbyName(name) != -1){
          write(getFDbyName(name), "haha xd\n", 1024);
        }

      } else if(validVerb("MSG ", receiveBuffer)){


        pid_t pid;

        memset(to, 0, 1024);
        strcpy(to, strtok(receiveBuffer, "MSG "));
        memset(from, 0, 1024);
        strcpy(from, strtok(NULL, " "));
        char msg[1024];
        strcpy(msg, strtok(NULL, ""));

      if(!strcmp(to, argv[1+numopt])){//incoming messages{}
          //writeToAuditLog(f, argv[1+numopt], "MSG", "from", from, msg);
      } else{ // out going messages
          writeToAuditLog(f, argv[1+numopt], "MSG", "to", to, msg);

      }


        if(!checkConnected(to, from)){ // not a pair  


        int fdpair[2];
        socketpair(AF_UNIX, SOCK_STREAM, 0, fdpair);
            //keeps tracks of connected users
            if(!strcmp(to, argv[1+numopt])){//incoming messages
              addPair(from, fdpair[0]);
            } else{ // out going messages
              addPair(to, fdpair[0]);
            }   


            if ((pid = fork()) ==-1)
              perror("fork error");
            else if (pid == 0) {  //forks and exec chat
              close(fdpair[0]);
              char fdss[1000];
              sprintf(fdss, "%d", fdpair[1]);


                if(!strcmp(to, argv[1+numopt])){//incoming messages
                  execl("/usr/bin/xterm", "/usr/bin/xterm", "-geometry", "45x25+500"
                    , "-T", to, "-e", "./chat", fdss, (void*)NULL);
                } else{ // out going messages
                  execl("/usr/bin/xterm", "/usr/bin/xterm", "-geometry", "45x25+100"
                    , "-T", from, "-e", "./chat", fdss, (void*)NULL); 
                }


             }
          else {//puts the stuff into the chat
                //close(fdpair[1]);//why need this?

              if(!strcmp(to, argv[1+numopt])){//incoming messages                  
              write(fdpair[0], from, 1024);
            } else{ // out going messages              
              write(fdpair[0], to, 1024);
            }

                if(!strcmp(to, argv[1+numopt]))
                  write(fdpair[0], ">", 1);      
                else
                  write(fdpair[0], "<", 1);                
          
                write(fdpair[0], msg, 1023);

                memset(receiveBuffer, 0, 1024);         
            } 
            printf("dsafdsfdsf\n");

            //maybe write who send to here         
        } else{// is currently a pair

                //close(fdpair[1]);//why need this?

            //multiplex chat stdin and writin

                if(!strcmp(to, argv[1+numopt])){
                  write(getFDbyName(from), ">", 1);      
                  write(getFDbyName(from), msg, 1023);
                }                  
                else{
                  write(getFDbyName(to), "<", 1);                
                  write(getFDbyName(to), msg, 1023);
                }
                  
          
        }
      }

//
    } else {
        userConnected * temp = connections;
        int i;
        while(temp != NULL){
          i = temp -> pair;
        

      if(FD_ISSET(i, &readfds)){
                  printf("i got in here\n");
// HAVE TO GO THROUGH FD IN CONNECT USERS TO DO MULTI CHAT PROPERLY

                  /* while loop maybe?
                      userConnected * temp = connections;
                      while(temp != NULL){
                      printf("%s\n", temp -> to);
                      temp = temp -> next;
                    }
                  */
      printf("%s   %s\n", to, from);
      memset(receiveBuffer, 0, 1024);
      
          Recv(i, receiveBuffer);  

      
      printf("but do i get into here?\n");
                      
      char name[1024];
      memset(name, 0, 1024);
      
          Recv(i, name); 
      
 

      name[strlen(name)-5] = '\0';


      if(!strcmp(receiveBuffer, "/close \r\n\r\n")){
          
        /*printf("\n%d       %d       %d\n", getFDbyName(name), getFDbyName(to), getFDbyName(from));
        printf("%s       %s       %s\n", name, to, (from));*/

          write(i, "/close", 1024);
          FD_CLR(i, &readfds);
          removePair(name);
  
        writeToAuditLog(f, argv[1+numopt], "CMD", "/close","success", "chat");
        justclosed = true;
        //write(1, (char*)3, 1);
        
        //close(fdpair[0]);
      } else {

        char msgBuffer[1024];
        memset(msgBuffer, 0, 1024);
        sprintf(msgBuffer, "MSG %s %s %s", name, argv[1+numopt], receiveBuffer);
        writeToAuditLog(f, argv[1+numopt], "MSG", "from", name, receiveBuffer);
      

      send(sockfd, msgBuffer, strlen(msgBuffer), 0);

      if(verbose)
        printf("\x1b[1;32m%s\x1b[0m\n", msgBuffer);
      //printf("lol\n");
      //printf("%s\n", msgBuffer);
     }
  }

temp = temp -> next;
        }

}
  //end of else



  }


}


return EXIT_SUCCESS;

}

void addPair(char * to, int pair){
    userConnected * temp = malloc(sizeof(userConnected));
    
    if(connections == NULL){

      connections = temp;
      connections -> next = NULL;
    } else{
      temp -> next = connections;
        connections = temp;  //update new head
    }
      temp -> to = malloc(sizeof(char*));
      strcpy(temp -> to, to);
      temp -> pair = pair;
      //printf("%s        %s\n", temp -> to, to);

}

void removePair(char * name){



   userConnected *temp, *prev;
  temp = connections;

  if (temp!= NULL){
    while (temp != NULL){
      if (strcmp(temp -> to, name) == 0){

        if (temp == connections){
          connections = temp -> next;
          break;

        }else{
          prev -> next = temp -> next;
          free (temp);
          break;
        }
      }else {
        prev = temp;
        temp = temp -> next;
      }
    }
}

}

void printAllConnection(){
   userConnected * temp = connections;
   while(temp != NULL){
    printf("%s\n", temp -> to);
    temp = temp -> next;
   }
      
  
}

int getFDbyName(char * name){
    userConnected * temp = connections;
   while(temp != NULL){
    if(!strcmp(temp -> to, name)){
      return temp -> pair;
    } 
    temp = temp -> next;
   }

   return -1;
      
}


void newUserProtocol(int sockfd, char* username, char **argv){
  char verb[110] = "WOLFIE \r\n\r\n";
  char receiveBuffer[1024];
  char iamnew [1024] = "IAMNEW ";
  char password[100];
  char newpass[128] = "NEWPASS ";
  char * getpassptr;



      //send 1 WOLFIE
  send(sockfd, verb, strlen(verb), 0);
  if(verbose)
  printf("\x1b[1;32m%s\x1b[0m\n", verb);


      //receive 1 EIFLOW
  memset(receiveBuffer, 0, 1024);
  Recv(sockfd, receiveBuffer);
  if(verbose)
  printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);


  if (strcmp(receiveBuffer, "EIFLOW \r\n\r\n") == 0){

    strcat(iamnew, username);
    strcat(iamnew, " \r\n\r\n");
    //send 2 IAMNEW <username>
    send(sockfd, iamnew, strlen(iamnew), 0);
    if(verbose)
    printf("\x1b[1;32m%s\x1b[0m\n", iamnew);

    //receive 2 HINEW <username>
    memset(receiveBuffer, 0, 1024);
    Recv(sockfd, receiveBuffer);
    if(verbose)
    printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);


    if (strstr(receiveBuffer, "ERR") == receiveBuffer){ //user name taken

        char ip[1024];
        memset(ip, 0, 1024);
        sprintf(ip, "%s:%s", argv[2+numopt], argv[3+numopt]);

      writeToAuditLog(f, username, "LOGIN", ip, "fail", receiveBuffer);

      memset(receiveBuffer, 0, 1024);
      Recv(sockfd, receiveBuffer);
      if(verbose)
      printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);

      printf("Terminating connection\n");
      close(sockfd);
      exit(0);

    }



    else{
//prompt user for password, send NEWPASS <password>
      getpassptr = getpass("Please enter a password\n");
      strcpy(password, getpassptr);

      strcat(newpass, password);
      strcat(newpass, " \r\n\r\n");

//send 3 NEWPASS <password>
      send(sockfd, newpass, strlen(newpass), 0);
      if(verbose)
      printf("\x1b[1;32m%s\x1b[0m\n", newpass);

//receive 3 SSAPWEN or ERR01
      memset(receiveBuffer, 0, 1024);
      Recv(sockfd, receiveBuffer);

      if(verbose)
      printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);

      if (strstr(receiveBuffer, "ERR") == receiveBuffer){ // bad password new user?

        char ip[1024];
        memset(ip, 0, 1024);
        sprintf(ip, "%s:%s", argv[2+numopt], argv[3+numopt]);
        writeToAuditLog(f, username, "LOGIN", ip, "fail", receiveBuffer);

        memset(receiveBuffer, 0, 1024);
        Recv(sockfd, receiveBuffer);
        if(verbose)
          printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);

        close(sockfd);
        exit(0);
        }

//receive HI <name>
//this gets stuck
      memset(receiveBuffer, 0, 1024);
      Recv(sockfd, receiveBuffer);
      if(verbose)
      printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);

        //receive MOTD
      memset(receiveBuffer, 0, 1024);
      Recv(sockfd, receiveBuffer);
      if(verbose)
        printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);


      if(verbose)
        printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);

        strtok(receiveBuffer, " "); //parse the motdverb
        char motdBuf[1024];
        strcpy(motdBuf, strtok(NULL, ""));
        printf("%s", motdBuf);
        char ip[1024];
        memset(ip, 0, 1024);
        sprintf(ip, "%s:%s", argv[2+numopt], argv[3+numopt]);
        writeToAuditLog(f, username, "LOGIN", ip, "success", motdBuf);



    }


    }else{
      printf("did not receive EIFLOW");
    }



                }

void existingUserProtocol(int sockfd, char*username, char** argv){
  char verb[110] = "WOLFIE \r\n\r\n";
  char receiveBuffer[1024];
  char iam[100]= "IAM ";
  char password[100];
  char *getpassptr;
  char pass[100] = "PASS ";

  //send 1 WOLFIE
  send(sockfd, verb, strlen(verb), 0);
  if(verbose)
  printf("\x1b[1;32m%s\x1b[0m\n", verb);

  //receive 1 EIFLOW
  memset(receiveBuffer, 0, 1024);
  Recv(sockfd, receiveBuffer);
  if(verbose)
  printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);


  if (strcmp(receiveBuffer, "EIFLOW \r\n\r\n") == 0){

    strcat(iam, username);
    strcat(iam, " \r\n\r\n");



    //send 2 IAM <username>
    send(sockfd, iam, strlen(iam), 0);
    if(verbose)
    printf("\x1b[1;32m%s\x1b[0m\n", iam);

    //receive 2 AUTH or ERR0 or ERR1
    memset(receiveBuffer, '\0', sizeof(receiveBuffer));

    Recv(sockfd, receiveBuffer);  //AUTH or ERR
    if(verbose)
    printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);

    if (strstr(receiveBuffer, "AUTH") == receiveBuffer){
    //if account was authorized, prompt user for password
      getpassptr = getpass("Please enter your password\n");

      strcpy(password, getpassptr);
      //scanf("%s", password);

      //send 3 PASS
      strcat(pass, password);
      strcat(pass, " \r\n\r\n");

      send(sockfd, pass, strlen(pass), 0);
      if(verbose)
      printf("\x1b[1;32m%s\x1b[0m\n", pass);


      //receive SSAP
      memset(receiveBuffer, 0, 1024);
      Recv(sockfd, receiveBuffer);
      if(verbose)
      printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);
    if(strstr(receiveBuffer, "SSAP ") == receiveBuffer){
      
      memset(receiveBuffer, 0, 1024);
      Recv(sockfd, receiveBuffer);
      if(verbose)
        printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);
      memset(receiveBuffer, 0, 1024);     
      Recv(sockfd, receiveBuffer);
      if(verbose) //MOTD
        printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);
      

     
      strtok(receiveBuffer, " ");
      char motdBuf[1024];
      strcpy(motdBuf, strtok(NULL, ""));
      printf("%s", motdBuf);

        char ip[1024];
        memset(ip, 0, 1024);
        sprintf(ip, "%s:%s", argv[2+numopt], argv[3+numopt]);
      writeToAuditLog(f, username, "LOGIN", ip, "success", motdBuf);


    } else if(strstr(receiveBuffer, "ERR ") == receiveBuffer){

        char ip[1024];
        memset(ip, 0, 1024);
        sprintf(ip, "%s:%s", argv[2+numopt], argv[3+numopt]);
        writeToAuditLog(f, username, "LOGIN", ip, "fail", receiveBuffer);

        memset(receiveBuffer, 0, 1024);
        Recv(sockfd, receiveBuffer);
        if(verbose)
          printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);

        memset(receiveBuffer, 0, 1024);
        close(sockfd);
        exit(0);

      }


    } else{

        char ip[1024];
        memset(ip, 0, 1024);
        sprintf(ip, "%s:%s", argv[2+numopt], argv[3+numopt]);
      writeToAuditLog(f, username, "LOGIN", ip, "fail", receiveBuffer);

      memset(receiveBuffer, 0, 1024);
      Recv(sockfd, receiveBuffer);
      if(verbose)
      printf("\x1b[1;32m%s\x1b[0m\n", receiveBuffer);

      close(sockfd);
      exit(0);
      }



  }else{
    printf("did not receive EIFLOW");
  }

}

void printHelpMenu(void){
  printf("\nUsage: \t./server [-hcv] NAME SERVER_IP SERVER_PORT\n");
  printf("-h \t\tDisplays this help menu and returns EXIT_SUCCESS.\n");
  printf("-c \t\tRequest to server to create a new user\n");
  printf("-v \t\tVerbose print all incoming and outgoing protocol verbs and content.\n");
  printf("NAME\tThis is the username to display when chatting.\n");
  printf("SERVER_IP\tThe ip address of the server to connect to.\n");
  printf("SERVER_PORT\tThe port number to connect to.\n");
}


bool validVerb(char * verb, char* message){
  bool returnB = false;
      //printf("this is %s %s %ld\n", verb, message, strlen(message));
      // second part of if statements checks if ends in \r\n\r\n
      // second part of if statements checks for no double space

  if(strstr(message, verb) == message  
    && !strcmp((char *)(message + strlen(message)-5), " \r\n\r\n") 
    && strcmp((char *)(message + strlen(message)-6), " ")){
    returnB = true;
  }

  return returnB;
}


void Recv(int fd, char * buffer){
  int i = 0; 

  while(1){
    recv(fd, buffer, 1, 0);
    //if(buffer[0] != 0){
      buffer++;
      i++;
    //}
      //printf("%d\t", buffer[-1]);
    if(strstr(buffer-i, " \r\n\r\n"))
      break;

  } 
      buffer = buffer - i;
}


bool checkConnected(char * to, char* from){
  userConnected * temp = connections;
  while(temp != NULL){

    if(( !strcmp(temp -> to, to) || !strcmp(temp -> to, from))){
        return true;
    }
            temp = temp -> next;

  }

  return false;
}




void sig_handler(int signo){
  if (signo == SIGINT){
    //printf("received SIGINT");
    logoutClient(verbose);
  } else
  if (signo == SIGCHLD){
      while (waitpid((pid_t)(-1), 0, WNOHANG | WUNTRACED) > 0) { 

       }

  }
}

void logoutClient(bool verbose){
  char *bye = "BYE \r\n\r\n";
  char buffer[1024];

  send(sockfd, bye, strlen(bye), 0);
  if(verbose)//
  printf("\x1b[1;32m%s\x1b[0m", bye);
  memset(buffer, 0, 1024);
  Recv(sockfd, buffer);
  if(verbose)
  printf("\x1b[1;32m%s\x1b[0m", buffer);

  if (strcmp(buffer, bye) == 0){

    printf("logging out\n");
    close(sockfd);
    exit(0);
  }


}



void writeToAuditLog(FILE * fd, char * name, char * verb, char * boo, char * msg, char * extra){

    char curStr[80];
    time_t curtime;
    time(&curtime);
    struct tm *info = localtime( &curtime );
    strftime(curStr,80,"%m/%d/%y-%I:%M%p", info);


    char buffer[2048];
    if(strlen(msg) == 0 && strlen(extra) == 0)
      sprintf(buffer, "%s, %s, %s, %s\n", curStr, name, verb, boo);
    else
      sprintf(buffer, "%s, %s, %s, %s, %s, %s\n", curStr, name, verb, boo, msg, extra);

    if(strstr(buffer, "\r\n\r\n"))
      buffer[strlen(buffer)-4] = '\0';
    if(buffer[strlen(buffer)-2] == '\n')
      buffer[strlen(buffer)-1] = 0;   // removes new line
    


    pthread_mutex_t sfwriteMutex;
    pthread_mutex_init(&sfwriteMutex, NULL);

  flock(fileno(f), LOCK_EX);
    sfwrite(&sfwriteMutex, fd, buffer);

  flock(fileno(f), LOCK_UN);



}