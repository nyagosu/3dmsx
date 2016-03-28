#ifndef DSFILELIST_H
#define DSFILELIST_H

#include "llist.h"

typedef struct tagFILELIST {
	int type;
	char name[256];
} FILELIST;

int fileselect( char * p, char * ext, char * fname );
FILELIST * getlist( LLIST * lst, int cnt );

#endif
