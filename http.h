#ifndef HTTP_H
#define HTTP_H

FILE * proc_req(char*);
char * build_response(FILE *, int);
int fptostr(char *, long, FILE *);

#endif
