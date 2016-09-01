#ifndef LLIST_H
#define LLIST_H

#define LLIST_OK	0
#define LLIST_ERR	-1
#define LLIST_TOL	-2
#define LLIST_EOL	-3

typedef struct tagListNode {
	struct tagListNode * next;
	struct tagListNode * prev;
	void * object;
} ListNode;

typedef struct tagList {
	int count;
	ListNode * top;
	ListNode * end;
	ListNode * cur;
} LLIST;

LLIST * LLIST_new();
void LLIST_free(LLIST * l);
int LLIST_ins( LLIST * l, void * obj );
int LLIST_top( LLIST * l );
int LLIST_end( LLIST * l );
int LLIST_add( LLIST * l, void * obj );
int LLIST_next( LLIST * l );
int LLIST_prev( LLIST * l );
int LLIST_count( LLIST * l );
void * LLIST_get( LLIST * l );
int LLIST_set( LLIST * l, void * obj );

#endif /* LLIST_H */
