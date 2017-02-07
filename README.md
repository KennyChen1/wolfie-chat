Full scale chat system written in C, built by Michael Dadurian and Kenny Chen.

Overview:

Clients attempt to establish a connection with the server through the Accept thread. The Accept thread will create a connection with the Client and spawn the Login thread. The Login thread then communicates with the client via the Wolfie Protocol to attempt to log the user into the Wolfie Chat system. Upon successful/unsuccessful login, the Login thread terminates. A Login thread is created for each connection attempt. Multiple Login threads can exist simultaneously, so a Login Queue was implemented to avoid concurrency issues.

Once logged in, the server acts as a middle man for all communications between the users connected to the server. A Communication thread on the server will be spawned when the first user logs in, and then terminates when no user is logged in. The Communication thread services all additional users who connect.

The Accept thread uses I/O multiplexing to listen for input on both the server socket and from stdin. We chose to use the "select" interface, see http://linux.die.net/man/2/select. In the server, we multiplex on each socket for the connected users in the communication thread, and in the accept thread we multiplex on the accept socket and stdin.


WOLFIE Protocol:

The server and client communicate with each other using the WOLFIE Protocol. 

For example, logging in to the server: When the server accepts a client connection request, the client initiates the login transaction by sending the "WOLFIE \r\n\r\n" verb to the server. The server responds to the client with "EIFLOW \r\n\r\n". The client identifies itself with the "IAM" verb. If login is successful, the server responds with "HI <name> \r\n\r\n", and the message of the day with the "MOTD <message> \r\n\r\n" verb. If login is unsuccessful, the server sends the "ERR" verb followed by the <errorcode> and the corresponding <message>.

Whatever verb the client sends to the server, that same verb backwards will be sent back to the client.
List of all verbs:
  "WOLFIE", "EIFLOW"
  "IAM <name>", "HI <name>", "BYE"
  "MOTD <message>", "ERR <errorcode> <message>"
  "TIME", "EMIT <timeinsec>"
  "LISTU", "UTSIL <user1> <user2>..",
  "MSG <TO> <FROM> <MESSAGE>"
  "UOFF <name>"
  
  Register new user: "IAMNEW <name>", "HINEW <name>", "NEWPASS <password>, "SSAPWEN"   
  Authenticate existing user: "AUTH <name>, PASS <password>, "SSAP"

Server usage and commands:

./server [-h|-v] PORT_NUMBER MOTD [ACCOUNTS_FILE]
-h              Displays help menu and returns EXIT_SUCCESS.
-v              Verbose print all incoming and outgoing protocol verbs and content.
PORT_NUMBER     Port number to listen on.
MOTD            Message to display to the client when they connect.
ACCOUNTS_FILE   File containing username and password data to be loaded upon execution.


/users : Dumps a list of currently logged in users to stdout.
/help : Lists all commands which the server accepts and what they do.
/shutdown : Server cleanly disconnects all connected users.
/accts : Dumps a list of all user accounts and information.

The server uses two linked lists, a Users list and an Accounts list. The Users list is a list of all u sers currently logged into the server. The Accounts list is the persistent list of users and their passwords loaded and saved to a file on the server's execution and termination respectively. 


Client usage and commands:

./client [-hcv] NAME SERVER_IP SERVER_PORT
-h            Displays this help menu, and returns EXIT_SUCCESS
-c            Requests to server to create a new user
-v            Verbose print all incoming and outgoing protocol verbs and content.
NAME          This is the username to display when chatting.
SERVER_IP     The ip address of the server to connect to.
SERVER_PORT   The port to connect to.


/time : Asks the server how long the client has been connected. The duration of the connection is returned in seconds, and the client program converts this time to hours, minutes, and seconds.
/help : Lists all commands which the client accepts.
/logout : Disconnect with the server, closes all chat windows.
/listu : Asks the server who has been connected.
/chat <to> <msg> : Requires the name of the user to send the message to and the message to be sent.


Chat Program:

./chat UNIX_SOCKET_FD
UNIX_SOCKET_FD        The Unix Domain File Descriptor number.

When the server sends the MSG verb to the client, it forks and execs an xterm (http://manpages.ubuntu.com/manpages/wily/end/man1/xterm.1.html), which in turn execs the chat program in a basic terminal. We used Unix domain sockets to communicate with the parent client process, using socketpair(2) (see http://linux.die.net/man/3/socketpair).



Password Criteria:

At least 5 characters in length, at least 1 uppercase character, at least 1 symbol character, and at least 1 number character.

User Authentication:

To protect passwords, we store a 1-way crytographic hash of the password. When a user attempts to login, the password is sent over the network, and is hashed by the server. The server will compare the stored hashed value with the hash value it just generated. To help defend against a cracked password, we add salt to everyone's password before hashing it. We used the hash function sha256, and the rand function to generate the salt (https://wwww.openssl.org/docs/manmaster/crypto/rand.html)


Error Codes:

00  USER NAME TAKEN
01  USER NOT AVAILABLE
02  BAD PASSWORD
100 INTERNAL SERVER ERROR(default)


