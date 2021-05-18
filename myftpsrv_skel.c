#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <err.h>

#include <netinet/in.h>

#define BUFSIZE 512
#define CMDSIZE 4
#define PARSIZE 100

#define MSG_220 "220 srvFtp version 1.0\r\n"
#define MSG_331 "331 Password required for %s\r\n"
#define MSG_230 "230 User %s logged in\r\n"
#define MSG_530 "530 Login incorrect\r\n"
#define MSG_221 "221 Goodbye\r\n"
#define MSG_550 "550 %s: no such file or directory\r\n"
#define MSG_299 "299 File %s size %ld bytes\r\n"
#define MSG_226 "226 Transfer complete\r\n"

/**
 * function: receive the commands from the client
 * sd: socket descriptor
 * operation: \0 if you want to know the operation received
 *            OP if you want to check an especific operation
 *            ex: recv_cmd(sd, "USER", param)
 * param: parameters for the operation involve
 * return: only usefull if you want to check an operation
 *         ex: for login you need the seq USER PASS
 *             you can check if you receive first USER
 *             and then check if you receive PASS
 **/
bool recv_cmd(int sd, char *operation, char *param) {
    char buffer[BUFSIZE], *token;
    int recv_s;

    // receive the command in the buffer and check for errors
    if((recv_s = read(sd, buffer, BUFSIZE)) < 0){
        warn("error reading buffer");
        return false;
    }
    if(recv_s == 0) {
        warn("empty buffer");
        return false;
    }
	


    // expunge the terminator characters from the buffer
    buffer[strcspn(buffer, "\r\n")] = 0;

    // complex parsing of the buffer
    // extract command receive in operation if not set \0
    // extract parameters of the operation in param if it needed
    token = strtok(buffer, " ");
    if (token == NULL || strlen(token) < 4) {
        warn("not valid ftp command");
        return false;
    } else {
        if (operation[0] == '\0') strcpy(operation, token);
        if (strcmp(operation, token)) {
            warn("abnormal client flow: did not send %s command", operation);
            return false;
        }
        token = strtok(NULL, " ");
        if (token != NULL) strcpy(param, token);
    }
    return true;
}

/**
 * function: send answer to the client
 * sd: file descriptor
 * message: formatting string in printf format
 * ...: variable arguments for economics of formats
 * return: true if not problem arise or else
 * notes: the MSG_x have preformated for these use
 **/
bool send_ans(int sd, char *message, ...){
    char buffer[BUFSIZE];

    va_list args;
    va_start(args, message);

    vsprintf(buffer, message, args);
    va_end(args);
    // send answer preformated and check errors
	if(!write(sd, buffer, strlen(buffer))){
            warn("Error");
            return false;
	}
	return true;


}

/**
 * function: RETR operation
 * sd: socket descriptor
 * file_path: name of the RETR file
 **/

/*void retr(int sd, char *file_path) {
    FILE *file;    
    int bread;
    long fsize;
    char buffer[BUFSIZE];

    // check if file exists if not inform error to client

    // send a success message with the file length

    // important delay for avoid problems with buffer size
    sleep(1);

    // send the file

    // close the file

    // send a completed transfer message
}
*/
/**
 * funcion: check valid credentials in ftpusers file
 * user: login user name
 * pass: user password
 * return: true if found or false if not
 **/
/*bool check_credentials(char *user, char *pass) {
    FILE *file;
    char *path = "./ftpusers", *line = NULL, cred[100];
    size_t len = 0;
    bool found = false;

    // make the credential string
    strcpy(cred, user);
    strcat(cred, ":");
    strcat(cred, pass);
    strcat(cred, "\n");
    // check if ftpusers file it's present

    // search for credential string

    // close file and release any pointes if necessary

    // return search status
}*/

/**
 * function: login process management
 * sd: socket descriptor
 * return: true if login is succesfully, false if not
 **/
/*bool authenticate(int sd) {
    char user[PARSIZE], pass[PARSIZE];

    // wait to receive USER action
	if(!recv_cmd(sd, "USER", user)){
		warnx("Failed to receive username");
		return false;
	}
    // ask for password
	if(!send_ans(sd, MSG_331, user)){
		warnx("Failed to ask password");
		return false;
	}
    // wait to receive PASS action
    if(!recv cmd(sd, "PASS", pass)){
		warnx("Failed to receive password");
		return false;
	}
    // if credentials don't check denied login or confirm login
	if(check_credentials(user, pass)) {
		if(!send_ans(sd, MSG_230, user)){
			warnx("failed to send login confirm");
			return false;
		}
		return true;
	}else{
		if(!send_ans(sd, MSG_530))
			warnx("failed to send login confirm");
		return false;
	}
}*/

/**
 *  function: execute all commands (RETR|QUIT)
 *  sd: socket descriptor
 **/

void operate(int sd) {
    char op[CMDSIZE], param[PARSIZE];

    while (true) {
        op[0] = param[0] = '\0';
        // check for commands send by the client if not inform and exit
	if(!recv_cmd(sd, op, param)){
	    warn("abnormal flow");
            continue;
        }

        if (strcmp(op, "RETR") == 0) {
            //retr(sd, param);
        } else if (strcmp(op, "QUIT") == 0) {
            // send goodbye and close connection
            send_ans(sd, MSG_221, NULL);
            close(sd);
            break;
        } else {
            // invalid command
            warn("command not supported");
            // furute use
        }
    }
}

/**
 * Run with
 *         ./mysrv <SERVER_PORT>
 **/
int main (int argc, char *argv[]) {

    // arguments checking
    if (argc != 2) {
        printf("1 argument needed to execute/n");
        exit(1);
    }
    
    // reserve sockets and variables space
    int master_sd, slave_sd;
    struct sockaddr_in master_addr, slave_addr;
    socklen_t slave_addr_len;
	
    // create server socket and check errors
    if((master_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Can't create server socket");
        exit(1);
    }
    // bind master socket and check errors
    master_addr.sin_family = AF_INET;
    master_addr.sin_addr.s_addr = INADDR_ANY;
    master_addr.sin_port = htons(atoi(argv[1]));
   
    if(bind(master_sd, (struct sockaddr *)&master_addr, sizeof(master_addr)) < 0) {
        perror("Can't bind socket");
        exit(2);
    }
    // make it listen
    if(listen(master_sd, 10) < 0) {
        perror("Listen error");
        exit(3);
    }
    // main loop
    while (true) {
        // accept connectiones sequentially and check errors
	slave_addr_len = sizeof(slave_addr);
	slave_sd = accept(master_sd, (struct sockaddr *)&slave_addr, &slave_addr_len);
	if(slave_sd < 0) {
	    errx(4, "Accept error");
	}
	#if DEBUG
	printf("Cliente se conecto\n");
	#endif //DEBUG
        // send hello
        send_ans(slave_sd, MSG_230);
        // operate only if authenticate is true
    }

    // close server socket
    close(master_sd);
    return 0;
}
