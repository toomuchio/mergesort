#include "libutil.h"

/*Safe malloc*/
void *safe_malloc(size_t size) {
	void *memory = NULL;

	/*Attempt to assign memory*/
	if ((memory = calloc(1, size)) == NULL) {
		printf("Fatal Error: Memory allocation failure in safe_malloc()\n");
		exit(EXIT_FAILURE);
	}

	return memory;
}

/*Insert an element into the pqueue sorting on its priority*/
void pqueue_insert(list_t *pqueue, data_t *payload, int offset) {
	node_t *newNode = safe_malloc(sizeof(node_t)), *currentNode = pqueue->head, *previousNode = NULL;

	/*Assign payload to node*/
	newNode->next = NULL;
	newNode->payload = payload;
	newNode->offset = offset;

	/*Find a place to insert the new node*/
	while(currentNode != NULL && currentNode->payload->GuildID < newNode->payload->GuildID) {
		previousNode = currentNode;
		currentNode = currentNode->next;
	}

	/*Insert the new node*/
    if (previousNode == NULL) {
		pqueue->head = newNode;
    } else {
		previousNode->next = newNode;
    }
	newNode->next = currentNode;
}

/*Dequeue the record with the highest priority (the head record)*/
void *pqueue_dequeue(list_t *pqueue) {
	node_t *currentNode = pqueue->head;

	/*If the head exists remove it and return*/
	if(currentNode != NULL) {
		pqueue->head = currentNode->next;
		return currentNode;
	} else {
		return NULL;
	}
}

/*Quicksort Algorithm - left = 0, right = n-1.*/
void quicksort(data_t **unsortedArray, int left, int right) {
	int x = left, y = right;
	data_t *tmp = NULL, *pivot = unsortedArray[(left + right) / 2]; /*Middle*/

	while(x <= y) {
		while(unsortedArray[x]->GuildID < pivot->GuildID) {
			x++;
		}

		while(unsortedArray[y]->GuildID > pivot->GuildID) {
			y--;
		}

		if(x <= y) { /*Perform Swap*/
			tmp = unsortedArray[x];
			unsortedArray[x] = unsortedArray[y];
			unsortedArray[y] = tmp;
			x++;
			y--;
		}
	}

	if(left < y) {
		quicksort(unsortedArray, left, y);
	}

	if(x < right) {
		quicksort(unsortedArray, x, right);
	}
}

