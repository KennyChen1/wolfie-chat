#define _GNU_SOURCE

#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>  
#include <unistd.h> 
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>








#define MAX_INPUT 1024

typedef struct userData{
	char * username;
	time_t connectTime;
	char * password;
	struct userData *next;
	int fd;
	char * filename; //need to send filename to login thread
	bool fileProvided; //need to send to login thread
} userData;

typedef struct accounts{
	char * username;
	char * password;
	unsigned char *salt;
	unsigned char *hash;
	struct accounts *next;
} accountsList;

typedef struct loginQ{
	userData sendData; //data to send to loginthread
	struct loginQ *next;

}loginQueue;




void printHelpMenu();
void printVerbose();
int handleArguements(int argc, char ** argv);
void printServerHelpMenu();
void *loginThread(void *vargp);
void *communicationThread(void *vargp);
char * printAllUsers(int boo);
void addUser(char *username, int fd, time_t connectTime, char * password);
void printUserInfo (userData *list);
char* checkProtocolPart1(int childfd);
void sendError (int childfd, int errorNo, char * user);
bool checkUsername(int childfd, char *username);
int removeUser(char *username);
void writeAccount(char * filename, char *username, unsigned char *salt, unsigned char * hash);
void addAccount(char *username, char *password, unsigned char * salt, unsigned char * hash);
void printAccountList();
FILE* loadFile(char * filename);
void loadFileIntoList (char * filename, FILE* fp);
bool validateAccountName(int childfd, char * username, bool newUser, char * motd, bool fileProvided);
bool newOrExisting (int childfd);
char* receiveAndCheckPassword (int childfd);
void completeLogin(int childfd, char* username);
bool validatePassword (char * password);
char* getUsername();
char * getTime(int fd);
bool checkExistingPassword (char * user, char* password);;
unsigned char * hashPassword(char * password, unsigned char * salt);
void Recv(int fd, char * buffer);
void sig_handler(int signo);
void shutdownServer(int parentfd);
void Enqueue(userData sendData);
void Dequeue();
void printLoginQ();