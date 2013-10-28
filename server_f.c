#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "http.h"
#include "io.h"
#include "server_f.h"

static void usage()
{
	extern char * __progname;
	fprintf(stderr, "usage: %s portnumber serv_directory log_directory\n", __progname);
	exit(1);
}

static void childhandler(int signum) {
	/* signal handler for SIGCHLD */
	waitpid(WAIT_ANY, NULL, WNOHANG);
}


int main(int argc,  char *argv[]) {
	struct sockaddr_in sockname, client;
	char buffer[80], *ep;
	struct sigaction sa;
	int clientlen, sd;
	u_short port;
	pid_t pid;
	u_long p;

	/*
 	 * first, figure out what port we will listen on - it should
	 * be our first parameter.
	 */
	if (argc != 4) {
		usage();
	}
	errno = 0;
	p = strtoul(argv[1], &ep, 10);
	if (*argv[1] == '\0' || *ep != '\0') {
		/* parameter wasn't a number, or was empty */
		fprintf(stderr, "%s - not a number\n", argv[1]);
		usage();
	}
	if ((errno == ERANGE && p == ULONG_MAX) || (p > USHRT_MAX)) {
		/* It's a number, but it either can't fit in an unsigned
		 * long, or is too big for an unsigned short
		 */
		fprintf(stderr, "%s - value out of range\n", argv[1]);
		usage();
	}
	/* now safe to do this */
	port = p;

	memset(&sockname, 0, sizeof(sockname));
	sockname.sin_family = AF_INET;
	sockname.sin_port = htons(port);
	sockname.sin_addr.s_addr = htonl(INADDR_ANY);
	sd = socket(AF_INET,SOCK_STREAM,0);
	if ( sd == -1) {
		err(1, "socket failed");
	}
	if (bind(sd, (struct sockaddr *) &sockname, sizeof(sockname)) == -1) {
		err(1, "bind failed");
	}

	if (listen(sd,3) == -1) {
		err(1, "listen failed");
	}

	/*
	 * we're now bound, and listening for connections on "sd" -
	 * each call to "accept" will return us a descriptor talking to
	 * a connected client
	 */


	/*
	 * first, let's make sure we can have children without leaving
	 * zombies around when they die - we can do this by catching
	 * SIGCHLD.
	 */
	sa.sa_handler = childhandler;
	sigemptyset(&sa.sa_mask);
	/*
	 * we want to allow system calls like accept to be restarted if they
	 * get interrupted by a SIGCHLD
	 */
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
		err(1, "sigaction failed");

	/*
	 * finally - the main loop.  accept connections and deal with 'em
	 */
	printf("Server up and listening for connections on port %u\n", port);
	while(1) {
		int clientsd;
		clientlen = sizeof(&client);
		clientsd = accept(sd, (struct sockaddr *)&client, &clientlen);
		if (clientsd == -1)
			err(1, "accept failed");
		/*
		 * We fork child to deal with each connection, this way more
		 * than one client can connect to us and get served at any one
		 * time.
		 */

		pid = fork();
		if (pid == -1)
			err(1, "fork failed");
			/*TODO send internal server error*/
		if(pid == 0) {
			char * request;
			char * response;
			FILE * file;
			printf("calling sockread\n");
			request = sockread(clientsd);
			fflush(stdout);
			printf("calling proc_req");
			file = proc_req(request);
			printf("entering if statement");
			if (file == 0) {
				printf("NULL received");
				response = build_response(file, errno);
			} else {
				printf("something returned");
				response = build_response(file, 0);
			}
			sockwrite(clientsd, response);
			exit(0);
		}
		close(clientsd);
	}
}
