#include <stdio.h>
#include "http.h"

int main() {
	char * astring;
	FILE * file;

	astring = "GET
	html/index.htm HTTP/1.1\njfkdla;jfd\nfkldsjf\n";
	file = proc_req(astring);
	astring = build_response(file, file == NULL);
	printf("%s", astring);

	return 0;
}
