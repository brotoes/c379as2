#ifndef HTTP_H
#define HTTP_H

FILE * proc_req(char*);
char * build_response(FILE *, int);
char * fptostr(int *, FILE *);
char * make_str(FILE *, const char *);

#endif
