#ifndef _LIBUTIL_H_
#define _LIBUTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/*Structure for Characters*/
typedef struct characters_t {
	char CName[40];
	unsigned short int Team;
	unsigned short int Level;
	unsigned int CId;
} characters_t;

typedef struct data_t {
	unsigned int GuildID;
	union type {
		char GName[40];
		characters_t cdata;
	} type;
} data_t;

/*Structure for Linked List.*/
typedef struct node_t {
	data_t *payload;
	int offset;
	struct node_t *next;
} node_t;

typedef struct {
	struct node_t *head;
} list_t;

/*Function Prototypes*/
void *safe_malloc (size_t size);
void pqueue_insert(list_t *list, data_t *payload, int offset);
void *pqueue_dequeue(list_t *list);
void quicksort(data_t **unsortedArray, int left, int right);

#endif

