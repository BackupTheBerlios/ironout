#ifndef _NAME_H
#define _NAME_H

#include "ast.h"
#include "cfile.h"
#include "project.h"

#define NAME_VAR	0x0001
#define NAME_PARAM	0x0002
#define NAME_FIELD	0x0004
#define NAME_ENUMVAL	0x0008
#define NAME_FUNCTION	0x0010
#define NAME_TYPEDEF	0x0020
#define NAME_PARAMDECL	0x0040

#define NAME_ENUM	0x0100
#define NAME_STRUCT	0x0200
#define NAME_UNION	0x0400
#define NAME_LABEL	0x0800

#define NAME_STATIC	0x1000
#define NAME_EXTERN	0x2000

#define NAME_MOD_MASK	0x0F00

struct name {
	char *name;
	int flags;
};

struct project;
struct cfile;

struct name *name_init(struct node *node, int flags);
struct name *name_find(struct project *project,
		       struct cfile *cfile, char *location);
void name_free(struct name *name);
int guess_name_flags(struct node *node);
int modifiers_match(struct name *name, int flags);

#endif
