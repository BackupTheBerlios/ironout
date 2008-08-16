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

int startswith(char *heystack, char *needle)
{
	return strstr(heystack, needle) == heystack;
}

int endswith(char *heystack, char *needle)
{
	return strstr(heystack, needle) ==
		heystack + strlen(heystack) - strlen(needle);
}
