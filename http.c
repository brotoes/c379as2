#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "io.h"
#include "http.h"

#define LINE_LEN 80

/*defined here to guarantee there won't be memory 
 *errors when generating error responses*/
static const char * efile_bad_req = "html/bad_request.htm";
static const char * efile_forb = "html/forbidden.htm";
static const char * efile_internal = "html/internal_error.htm";
static const char * efile_notfound = "html/not_found.htm";

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
	char * response;
	
	if (error == 0) {
		response = make_str(file, "HTTP/1.1 200 OK\n");
		/*TODO implement and call logging function*/
	} else {
		FILE * efile;

		switch (errno) {
			case EBADRQC:
			case ENOSYS:
			case EPFNOSUPPORT: {
				/*Bad Request Error*/
				efile = fopen(efile_bad_req, "r");
				response = make_str(efile,
					"HTTP/1.1 400 Bad Request\n");
				/*TODO implement and call logging function*/

			} break;
			case EACCES: {
				/*Forbidden Error*/
				efile = fopen(efile_forb, "r");
				response = make_str(efile,
					"HTTP/1.1 400 Forbidden\n");
				/*TODO implement and call logging function*/
			} break;
			case ENOENT: {
				/*Not Found Error*/
				efile = fopen(efile_notfound, "r");
				response = make_str(efile,
					"HTTP/1.1 404 Not Found\n");
				/*TODO implement and call logging function*/
			} break;
			default: {
				/*Internal Error*/
				efile = fopen(efile_internal, "r");
				response = make_str(efile, 
					"HTTP/1.1 500 Internal Server Error\n");
				/*TODO implement and call logging function*/
			}
		}
	}

	return response;
}

/*
 *returns string filled with the contents of FILE file
 *
 *puts number of bytes read into int read
 *returns NULL if error reading
 */
char * fptostr(int * bytes_read, FILE * file) {
	long rc;
	char * temp;
	int fsize;
	int read;
	char * content;

	fseek(file, 0L, SEEK_END);
	fsize = ftell(file);
	fseek(file, 0L, SEEK_SET);

	content = malloc(fsize*sizeof(char));

	for (read = 0; read < fsize; read += rc) {
		if ((rc = fread((void*)(content + read), 
			sizeof(char), fsize, file)) == -1) {
			if (errno != EINTR) {
				return NULL;
			} else {
				rc = 0;
			}
		}
	}

	*bytes_read = read;
	return content;
}

char * make_str(FILE * file, const char * msg) {
	char * temp;
	char * date;
	char * content;
	char * content_len = malloc(LINE_LEN*sizeof(char));
	char * content_type = malloc(LINE_LEN*sizeof(char));
	char * response;
	char * head = malloc(LINE_LEN*sizeof(char));
	time_t raw_time;
	struct tm * time_info;
	long fsize;
	int read;

	strlcpy(head, msg, LINE_LEN);

	time(&raw_time);
	time_info = gmtime(&raw_time);
	strftime(date, LINE_LEN, "Date: %a, %d %b %Y %T GMT\n", time_info);

	strlcpy(content_type, "Content-Type: text/html\n", LINE_LEN);

	fseek(file, 0L, SEEK_END);
	fsize = ftell(file);
	fseek(file, 0L, SEEK_SET);
	response = malloc(LINE_LEN*4 + fsize);

	content = fptostr(&read, file);

	snprintf(content_len, LINE_LEN, "Content-Length: %ld\n\n", fsize);

	snprintf(response, fsize + LINE_LEN*4, "%s%s%s%s%s\0", head, date, 
		 content_type, content_len, content);
	
	return response;
}
