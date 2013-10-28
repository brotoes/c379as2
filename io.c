#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "io.h"

/*
 *writes to fd the string contained in msg
 *returns 0 on success
 */
int sockwrite(int fd, char * msg) {
	ssize_t written = 0;
	ssize_t w = 0;

	while (written < strlen(msg)) {
		w = write(fd, msg + written, 
			  strlen(msg) - written);
		if (w == -1) {
			if (errno != EINTR) {
				err(1, "write failed\n");
			}
		} else {
			written += w;	
		}
	}

	return 0;
}

/*
 *reads until \r\n\r\n or \n\n into a string pointed to by result
 *returns 0 on success
 *sets result to NULL on error
 */
char * sockread(int fd) {
	ssize_t bytes_read = -1;
	ssize_t rc = 0;
	int maxread = 4096;
	char * result = malloc(maxread*sizeof(char));
	int http_fin = 0;
	char last_read[4];

	while ((bytes_read != 0) && rc < maxread
		&& !http_fin) {
		int i;
		bytes_read = read(fd, result + rc, maxread - rc);
		if (bytes_read == -1) {
			if (errno != EINTR) {
				err(1, "read failed\n");
			}
		} else {
			rc += bytes_read;
		}
		/*if a blank line has been sent, set flag*/
		for (i = 0; i < 4; i ++) {
			last_read[i] = result[rc - 4 + i];
			printf("%c", last_read[i]);
		}
		if ((last_read[0] == '\r' && last_read[1] == '\n' &&
		    last_read[2] == '\r' && last_read[3] == '\n') || 
		    (last_read[2] == '\n' && last_read[3] == '\n')) {
			http_fin = 1;
			printf("http flag set\n");
		}
	}
	printf("loop exited\n");
	result[rc] = '\0';
	printf("returned %s\n", result);
	return result;
}
