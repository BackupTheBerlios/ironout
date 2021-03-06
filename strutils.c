#include <ctype.h>
#include <string.h>


/* returns the start of the next token */
char *readtoken(char *to, char *from, char *delims)
{
	for (; *from != '\0' && !strchr(delims, *from);)
		*to++ = *from++;
	*to = '\0';
	for (; *from != '\0' && strchr(delims, *from);)
		from++;
	return from;
}

/* returns the start of the next word */
char *readword(char *to, char *from)
{
	for (; *from != '\0' && isalnum(*from);)
		*to++ = *from++;
	*to = '\0';
	for (; *from != '\0' && !isalnum(*from);)
		from++;
	return from;
}

char *nthtoken(char *to, char *from, char* delims, int n)
{
	int i = 0;
	for (i = 0; i < n; i++)
		from = readtoken(to, from, delims);
	return from;
}

int startswith(char *heystack, char *needle)
{
	while (*needle != '\0' && *needle == *heystack++)
		needle++;
	return !*needle;
}

int endswith(char *heystack, char *needle)
{
	char *p = heystack + strlen(heystack);
	char *q = needle + strlen(needle);
	while (q != needle && p != heystack && *--p == *--q)
		;
	return *p == *q && q == needle;
}
