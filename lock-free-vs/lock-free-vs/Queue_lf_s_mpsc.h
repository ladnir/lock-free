/*
*
*
* Created on: May 4, 2014
* Author: Peter Rindal
*
* Bounded single producer single consumer
* non-blocking lock free queue.
*
* g++ --std=c++11 ...
*/

#ifndef QUEUE_LF_S_MPSC_H_
#define QUEUE_LF_S_MPSC_H_
#include <stdlib.h>
#include "stdafx.h"

using namespace std;

struct pointer_guard{

	void* ptr;
	atomic<unsigned int> guard;
};

struct queue_lf_s_mpsc_t{

	struct pointer_guard* list;
	unsigned int cap;

	atomic<unsigned int> tail;
	atomic<unsigned int> head;

	int(*push)(struct queue_lf_s_mpsc_t *, void*);
	int(*pop)(struct queue_lf_s_mpsc_t *, void**);
};

int _push(struct queue_lf_s_mpsc_t * queue, void* entry){

	unsigned int tail, old_head;

	do{
		tail = queue->tail.load(memory_order_relaxed);
		old_head = queue->head.load(memory_order_relaxed);
		//printf("push tail %d, head %d\n", tail, old_head);
		if (tail + queue->cap - 1 <= old_head)return 0;

		// try and reserve head for our selves
	} while (!atomic_compare_exchange_weak_explicit(&queue->head,
		&old_head,
		old_head + 1,
		memory_order_release,
		memory_order_acquire));
	// the index of old_head is ours!

	struct pointer_guard * pg = &queue->list[old_head%queue->cap];
	pg->ptr = entry;
	pg->guard.store(1, memory_order_release);
	return 1;
}

int _pop(struct queue_lf_s_mpsc_t * queue, void ** dest){

	unsigned int lcl_tail = queue->tail.load(memory_order_relaxed);
	unsigned int lcl_head = queue->head.load(memory_order_relaxed);

	if (lcl_tail != lcl_head) { // not empty.

		struct pointer_guard * pg = &queue->list[lcl_tail%queue->cap];

		if (pg->guard.load(memory_order_acquire)){ // data is ready
			*dest = pg->ptr;
			pg->guard.store(0, memory_order_relaxed); // could be relaxed?
			queue->tail.store(lcl_tail + 1, memory_order_release);
			return 1; // success? haha
		}
		//printf("data not ready. tail %d , head %d , cap%d\n",tail,old_head,queue->cap);
	}
	else{
		//printf("empty. ");
	}

	// A second ago it was empty or the data wasn't ready.
	return 0;
}

void init_queue_lf_s_mpsc(struct queue_lf_s_mpsc_t * queue, unsigned int capacity){

	assert(capacity + 1 != 0 && "capacity must be less Max unsigned int");

	queue->list = (struct pointer_guard*) malloc(sizeof (struct pointer_guard) * (capacity + 1));
	queue->cap = capacity + 1;
	queue->tail.store(0, memory_order_relaxed);
	queue->head.store(0, memory_order_relaxed);

	queue->pop = _pop;
	queue->push = _push;
}


#endif /* QUEUE_LF_S_MPSC_H_ */