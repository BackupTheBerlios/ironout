#ifndef _STRUTILS_H
#define _STRUTILS_H

char *readtoken(char *to, char *from, char* delims);
char *readword(char *to, char *from);
int startswith(char *heystack, char *needle);
int endswith(char *heystack, char *needle);

#endif
