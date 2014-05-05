/*
 * queue_spsc.h
 *
 *  Created on: May 4, 2014
 *      Author: rindalp
 */
#include <stdlib.h>

#ifndef QUEUE_SPSC_H_
#define QUEUE_SPSC_H_



struct lf_queue_spsc_t{

	void * volatile * list;

	int cap;
	volatile int head;
	volatile int tail;

	int (*push)(struct lf_queue_spsc_t *, void *);
	void *(*pop)(struct lf_queue_spsc_t *);
};



int _push(struct lf_queue_spsc_t * queue, void * entry){

	//printf("head= %d, tail= %d\n",queue->head,queue->tail);
	sleep(1);
	if(queue->tail == (queue->head + 1) % queue->cap ){
		printf("full\n");

		return 0;
	}

	queue->list[queue->head] = entry;
	queue->head++;
	return 1;
}

void * _pop(struct lf_queue_spsc_t * queue){
	sleep(1);
	//printf("tail= %d, head= %d\n",queue->tail,queue->head);
	if(queue->tail == queue->head){
		printf("empty\n");
		return NULL;
	}

	void * entry = queue->list[queue->tail];
	queue->tail = (queue->tail + 1 ) %queue->cap;

	return entry;
}

void init_lf_queue_spsc(struct lf_queue_spsc_t * queue, int capacity){

	//queue =  (struct lf_queue_spsc_t*) malloc(sizeof(struct lf_queue_spsc_t));
	queue->list = malloc(sizeof (void*) * capacity);
	queue->cap = capacity;
	queue->head = 0;
	queue->tail = 0;

	queue->pop = _pop;
	queue->push = _push;
}
#endif /* QUEUE_SPSC_H_ */
