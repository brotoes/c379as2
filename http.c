#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "http.h"

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
 */
char * build_response(FILE * file, int error) {
	const char * efile_bad_req = "html/bad_request.htm";
	const char * efile_forb = "html/forbidden.htm";
	const char * efile_internal = "html/internal_error.htm";
	const char * efile_notfound = "html/not_found.htm";

	char * response;
	char * head = malloc(80*sizeof(char));
	char * date = malloc(80*sizeof(char));
	char * content_type = malloc(80*sizeof(char));
	char * content_len = malloc(80*sizeof(char));
	char * content;
	long size;
	time_t raw_time;
	struct tm * time_info;

	if (error == 0) {
		strlcpy(head, "HTTP/1.1 200 OK\n", 80);

		time(&raw_time);
		time_info = gmtime(&raw_time);
		strftime(date, 80, "Date: %a, %d %b %Y %T GMT\n", time_info);

		strlcpy(content_type, "Content-Type: text/html\n", 80);
	
		fseek(file, 0L, SEEK_END);
		size = ftell(file);
		fseek(file, 0L, SEEK_SET);

		content = malloc((size)*sizeof(char));
		response = malloc((size + 321)*sizeof(char));

		fptostr(content, size, file);
		
		snprintf(content_len, 80, "Content-Length: %ld\n\n", size);

		snprintf(response, size + 321, "%s%s%s%s%s\0", head, date, 
			 content_type, content_len, content);

		free(content);
	} else {
		FILE * efile;
		switch (errno) {
			case EBADRQC:
			case ENOSYS:
			case EPFNOSUPPORT: {
				/*Bad Request Error*/
				efile = fopen(efile_bad_req, "r");
				strlcpy(head, 
					"HTTP/1.1 400 Bad Request\n", 80);

				time(&raw_time);
				time_info = gmtime(&raw_time);
				strftime(date, 80, 
					"Date: %a, %d %b %Y %T GMT\n", 
					time_info);

				strlcpy(content_type, 
					"Content-Type: text/html\n", 80);
			
				fseek(efile, 0L, SEEK_END);
				size = ftell(efile);
				fseek(efile, 0L, SEEK_SET);

				content = malloc((size)*sizeof(char));
				response = malloc((size + 321)*sizeof(char));

				fptostr(content, size, efile);

				snprintf(content_len, 80, 
					"Content-Length: %ld\n\n", size);

				snprintf(response, size + 321, "%s%s%s%s%s\0",
					head, date, content_type,
					content_len, content);
				/*TODO implement and call logging function*/
				free(content);

			} break;
			case EACCES: {
				/*Forbidden Error*/
				efile = fopen(efile_bad_req, "r");
				strlcpy(head, 
					"HTTP/1.1 400 Bad Request\n", 80);

				time(&raw_time);
				time_info = gmtime(&raw_time);
				strftime(date, 80, 
					"Date: %a, %d %b %Y %T GMT\n", 
					time_info);

				strlcpy(content_type, 
					"Content-Type: text/html\n", 80);
			
				fseek(efile, 0L, SEEK_END);
				size = ftell(efile);
				fseek(efile, 0L, SEEK_SET);

				content = malloc((size)*sizeof(char));
				response = malloc((size + 321)*sizeof(char));

				fptostr(content, size, efile);

				snprintf(content_len, 80, 
					"Content-Length: %ld\n\n", size);

				snprintf(response, size + 321, "%s%s%s%s%s\0",
					head, date, content_type,
					content_len, content);
				/*TODO implement and call logging function*/
				free(content);
			} break;
			case ENOENT: {
				/*Not Found Error*/
				efile = fopen(efile_bad_req, "r");
				strlcpy(head, 
					"HTTP/1.1 400 Bad Request\n", 80);

				time(&raw_time);
				time_info = gmtime(&raw_time);
				strftime(date, 80, 
					"Date: %a, %d %b %Y %T GMT\n", 
					time_info);

				strlcpy(content_type, 
					"Content-Type: text/html\n", 80);
			
				fseek(efile, 0L, SEEK_END);
				size = ftell(efile);
				fseek(efile, 0L, SEEK_SET);

				content = malloc((size)*sizeof(char));
				response = malloc((size + 321)*sizeof(char));

				fptostr(content, size, efile);

				snprintf(content_len, 80, 
					"Content-Length: %ld\n\n", size);

				snprintf(response, size + 321, "%s%s%s%s%s\0",
					head, date, content_type,
					content_len, content);
				/*TODO implement and call logging function*/
				free(content);
			}break;
			default: {
				/*Internal Error*/
				efile = fopen(efile_bad_req, "r");
				strlcpy(head, 
					"HTTP/1.1 400 Bad Request\n", 80);

				time(&raw_time);
				time_info = gmtime(&raw_time);
				strftime(date, 80, 
					"Date: %a, %d %b %Y %T GMT\n", 
					time_info);

				strlcpy(content_type, 
					"Content-Type: text/html\n", 80);
			
				fseek(efile, 0L, SEEK_END);
				size = ftell(efile);
				fseek(efile, 0L, SEEK_SET);

				content = malloc((size)*sizeof(char));
				response = malloc((size + 321)*sizeof(char));

				fptostr(content, size, efile);

				snprintf(content_len, 80, 
					"Content-Length: %ld\n\n", size);

				snprintf(response, size + 321, "%s%s%s%s%s\0",
					head, date, content_type,
					content_len, content);
				/*TODO implement and call logging function*/
				free(content);
			}
		}
	}

	free(head);
	free(date);
	free(content_type);
	free(content_len);
	
	return response;
}

int fptostr(char * content, long size, FILE * file) {
	/*TODO do this more robustly*/
	return fread((void*)content, sizeof(char), size, file);
}
