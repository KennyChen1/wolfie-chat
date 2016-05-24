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
#include <sys/file.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define MAX_INPUT 1024

void printHelpMenu();
void printVerbose();
int handleArguements(int argc, char ** argv);
void printClientHelpMenu();

void createNewUserProtocol(int sockfd, char* username);
void existingUserProtocol(int sockfd, char*username, char** argv);
void newUserProtocol(int sockfd, char* username, char** argv);
void sendPassword(int sockfd);
bool validVerb(char * verb, char* message);
bool checkConnected(char * to, char* from);
void addPair(char * to, int pair);
void Recv(int sockfd, char * serverMessage);
char * getName(char* from);
void removePair(char* name);
void sig_handler(int signo);
void logoutClient(bool verbose);
void printAllConnection();
int getFDbyName(char * name);

void writeToAuditLog(FILE * fd, char * name, char * verb, char * boo, char * msg, char *extra);

typedef struct userConnected{
	char * to;
	struct userConnected *next;
	int pair;

} userConnected;