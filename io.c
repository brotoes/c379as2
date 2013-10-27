#include <errno.h>
#include "io.h"

/*
 *writes to fd the string contained in msg
 *returns 0 on success
 */
int sockwrite(int fd, char * msg) {
	ssize_t written = 0;
	ssize_t w = 0;

	while (written < strlen(buffer)) {
		w = write(clientsd, buffer + written, 
			  strlen(buffer) - written);
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
 *reads from fd into a string pointed to by result
 *returns 0 on success
 *sets result to NULL on error
 */
int sockread(int fd, char * result) {
	ssize_t r = -1;
	ssize_t rc = 0;
	size_t maxread;
	
	maxread = len - 1;
	while ((r != 0) && rc < maxread) {
		r = read(sd, buffer + rc, maxread - rc);
		if (r == -1) {
			if (errno != EINTR) {
				err(1, "read failed\n");
			}
		} else {
			rc += r;
		}
	}
	
	buffer[rc] = '\0';

	/*debug TODO REMOVE*/
	printf("Client Requested:\n%s", buffer);

	return 0;
}
