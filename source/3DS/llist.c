#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "llist.h"

static ListNode * newNode()
{
	ListNode * n;
	n = (ListNode *)malloc( sizeof( ListNode ) );
	n->next = NULL;
	n->prev = NULL;
	n->object = NULL;
	return n;
}

LLIST * LLIST_new()
{
	LLIST * l;
	l = (LLIST *) malloc( sizeof(LLIST) );
	l->count = 0;
	l->top = newNode();
	l->end = newNode();
	l->top->next = l->end;
	l->end->prev = l->top;
	l->cur = l->end;
	return l;
}

void LLIST_free(LLIST * l)
{
	ListNode * c;
	ListNode * n;
	if( l == NULL ) return;
	n = l->top;
	while( n != NULL ){
		c = n;
		n = n->next;
		if( c->object != NULL ) free( c->object );
		free( c );
	}
	free( l );
}

/* ƒJƒŒƒ“ƒg‚Ì‘O‚É’Ç‰Á */
int LLIST_ins( LLIST * l, void * obj )
{
	ListNode * n;
	if( l == NULL ) return LLIST_ERR;
	n = newNode();
	n->prev = l->cur->prev;
	n->next = l->cur;
	l->cur->prev->next = n;
	l->cur->prev       = n;
	l->cur             = n;
	l->cur->object     = obj;
	l->count ++;
	return LLIST_OK;
}

int LLIST_top( LLIST * l )
{
//	printf( "top\n");
	if( l == NULL ) return LLIST_ERR;
	l->cur = l->top->next;
	return LLIST_OK;
}

int LLIST_end( LLIST * l )
{
	if( l == NULL ) return LLIST_ERR;
	l->cur = l->end;
	return LLIST_OK;
}

/* ÅŒã‚É’Ç‰Á */
int LLIST_add( LLIST * l, void * obj )
{
//	LOG( "List Add" );
	if( LLIST_end( l ) == LLIST_ERR ) return LLIST_ERR;
	return LLIST_ins( l , obj );
}

int LLIST_next( LLIST * l )
{
//	printf( "next");
	if( l == NULL ) {
//		printf( " err\n");
		return LLIST_ERR;
	}
	if( l->cur->next == l->end ){
//		printf( " EOL\n");
		return LLIST_EOL;
	}
	l->cur = l->cur->next;
//	printf( " OK\n");
	return LLIST_OK;
}

int LLIST_prev( LLIST * l )
{
//	printf( "prev\n");
	if( l == NULL ) return LLIST_ERR;
	if( l->cur->prev == l->top ) return LLIST_TOL;
	l->cur = l->cur->prev;
	return LLIST_OK;
}


int LLIST_count( LLIST * l )
{
	if( l == NULL ) return -1;
	return l->count;
}

void * LLIST_get( LLIST * l )
{
//	printf( "get\n");
	if( l == NULL ) return NULL;
	if( l->cur == l->end ) return NULL;
	return l->cur->object;
}

int LLIST_set( LLIST * l , void * obj )
{
//	printf( "get\n");
	if( l == NULL ) return LLIST_ERR;
	if( l->cur == l->end ) return LLIST_ERR;
	if( l->cur->object )  free( l->cur->object );
	l->cur->object = obj;
	return LLIST_OK;
}

#if 0
int main( int ac, char **av )
{
	LLIST * l;
	char * buf;
	int ret;
	printf( "1\n" );
	l = LLIST_new();
	if( l == NULL ){
		printf( "LLIST_new err\n" );
		return 0;
	}
	printf( "LLIST_new OK\n" );
	buf = (char*)malloc( 13 );
	strcpy( buf, "aaaa" );
	LLIST_add( l, (void*)buf );
	buf = (char*)malloc( 13 );
	strcpy( buf, "bbb" );
	LLIST_add( l, (void*)buf );
	buf = (char*)malloc( 13 );
	strcpy( buf, "ccc" );
	LLIST_add( l, (void*)buf );
	buf = (char*)malloc( 13 );
	strcpy( buf, "ddd" );
	LLIST_add( l, (void*)buf );
	printf( "10\n" );
	
	LLIST_top( l );
	ret = LLIST_OK;
	printf( "4\n" );
	while( ret == LLIST_OK ){
		buf = (char*)LLIST_get( l );
		if( buf != NULL ){
			printf( "buf=>" );
			printf( "%s\n", buf );
		}
		ret = LLIST_next(l);
	}
	printf( "end\n");
	return 0;
}
#endif
