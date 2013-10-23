#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "http.h"

#define LINE_LEN 80

/*defined here to guarantee there won't be memory 
 *errors when generating error responses*/
static const char * efile_bad_req = "html/bad_request.htm";
static const char * efile_forb = "html/forbidden.htm";
static const char * efile_internal = "html/internal_error.htm";
static const char * efile_notfound = "html/not_found.htm";

static int contentarr_len = 4096;
static char response[4416];
static char head[LINE_LEN];
static char date[LINE_LEN];
static char content_type[LINE_LEN];
static char content_len[LINE_LEN];
static char content[4096];
static long size;
static time_t raw_time;
static struct tm * time_info;

/*accepts a string detailing an http request
 *returns an open FILE * pointer containing the file requested.
 *returns NULL if error is encountered, sets errno appropriately
 */
FILE * proc_req(char * req) {
	char * req_type = malloc(80*sizeof(char));
	char * filename = malloc(80*sizeof(char));
	char * protocol = malloc(80*sizeof(char));
	int n_items;
	FILE * file;

	/*interpret input*/
	n_items = sscanf(req, "%s %s %s\n", req_type, filename, protocol);
	if (n_items != 3) {
		errno = EBADRQC;
	}
	/*test request type*/
	if (strcmp(req_type, "GET")) {
		errno = ENOSYS;
		return NULL;
	}

	/*test protocol*/
	if (strcmp(protocol, "HTTP/1.1")) {
		errno = EPFNOSUPPORT;
		return NULL;
	}
	/*attempt to open file*/
	file = fopen(filename, "r");

	return file;
}

/*generates a response to a get request. accepts a file descriptor and
 *a variable that determines if error reponse will be generated
 *if error is not 0, errno will be used to genereate an error response
 *returns NULL in case of error
 *TODO: in case of error in here, find a way to return error packet
 */
char * build_response(FILE * file, int error) {
	int temp_len = contentarr_len;
	char * temp;
	if (error == 0) {
		make_str(file, "HTTP/1.1 200 OK\n");
		/*TODO implement and call logging function*/
	} else {
		FILE * efile;

		switch (errno) {
			case EBADRQC:
			case ENOSYS:
			case EPFNOSUPPORT: {
				/*Bad Request Error*/
				efile = fopen(efile_bad_req, "r");
				make_str(efile,
					"HTTP/1.1 400 Bad Request\n");
				/*TODO implement and call logging function*/

			} break;
			case EACCES: {
				/*Forbidden Error*/
				efile = fopen(efile_forb, "r");
				make_str(efile,
					"HTTP/1.1 400 Forbidden\n");
				/*TODO implement and call logging function*/
			} break;
			case ENOENT: {
				/*Not Found Error*/
				efile = fopen(efile_notfound, "r");
				make_str(efile,
					"HTTP/1.1 404 Not Found\n");
				/*TODO implement and call logging function*/
			} break;
			default: {
				/*Internal Error*/
				efile = fopen(efile_internal, "r");
				make_str(efile, 
					"HTTP/1.1 500 Internal Server Error\n");
				/*TODO implement and call logging function*/
			}
		}
	}

	
	return response;
}

/*returns as an int the number of bytes read. Will reallocate if it runs out
 *of space.
 *returns -1 if an error occured and could not read
 */
int fptostr(char * content, int * len, FILE * file) {
	long read;
	long rc;
	char * temp;
	int fsize;

	fseek(file, 0L, SEEK_END);
	fsize = ftell(file);
	fseek(file, 0L, SEEK_SET);

	/*if the file is longer than what is allocated for, realloc*/
	while (fsize > *len) {
		*len = (*len)*2;
		temp = realloc(content, *len);
		if (temp != NULL) {
			content = temp;
		}
	}

	for (read = 0; read < size; read += rc) {
		if ((rc = fread((void*)(content + read), 
			sizeof(char), size, file)) == -1) {
			if (errno != EINTR) {
				return -1;
			} else {
				rc = 0;
			}
		}
	}
	return read;
}

int make_str(FILE * file, const char * msg) {
	int temp_len = contentarr_len;
	char * temp;
	strlcpy(head, msg, LINE_LEN);

	time(&raw_time);
	time_info = gmtime(&raw_time);
	strftime(date, LINE_LEN, "Date: %a, %d %b %Y %T GMT\n", time_info);

	strlcpy(content_type, "Content-Type: text/html\n", 80);

	fseek(file, 0L, SEEK_END);
	size = ftell(file);
	fseek(file, 0L, SEEK_SET);

	fptostr(content, &temp_len, file);
	/*content got realloced, reallocate response*/
	if (temp_len != contentarr_len) {
		temp = realloc(response, temp_len + LINE_LEN*4);
		if (temp != NULL) {
			response = temp;
		}
		contentarr_len = temp_len;
	}

	snprintf(content_len, LINE_LEN, "Content-Length: %ld\n\n", size);

	snprintf(response, size + 321, "%s%s%s%s%s\0", head, date, 
		 content_type, content_len, content);
}
