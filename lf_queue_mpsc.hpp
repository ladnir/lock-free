/*
 * lf_queue_mpsc.hpp
 *
 *  Created on: May 4, 2014
 *      Author: Peter Rindal
 *
 *      Bounded single producer single consumer
 *      non-blocking lock free queue.
 *
 *      g++ --std=c++11 ...
 */

#ifndef LF_QUEUE_MPSC_HPP_
#define LF_QUEUE_MPSC_HPP_
#include <stdlib.h>
#include <atomic>
#include <thread>
#include <cassert>

#define EMPTY 0
#define FULL 1
#define PARTIAL 2

using namespace std;

struct pointer_guard{

	void* ptr;
	atomic<unsigned int> guard;
};

struct lf_queue_mpsc_t{

	struct pointer_guard* list;
	unsigned int cap;

	atomic<unsigned int> tail;
	atomic<unsigned int> head;

	int (*push)(struct lf_queue_mpsc_t *, void* );
	int (*pop )(struct lf_queue_mpsc_t *, void**);
};

int _push(struct lf_queue_mpsc_t * queue, void* entry){

	unsigned int tail, old_head;

	do{
		tail = queue->tail.load(memory_order_relaxed);
		old_head = queue->head.load(memory_order_relaxed);
		if( tail + queue->cap -1 <= old_head )return 0;

	// try and reserve head for our selves
	}while(! atomic_compare_exchange_weak_explicit(&queue->head,
										  	  	   &old_head,
										  	  	   old_head+1,
										  	  	   memory_order_release,
										  	  	   memory_order_acquire ));
	// the index of old_head is ours!

	struct pointer_guard * pg = &queue->list[old_head%queue->cap];
	pg->ptr = entry;
	pg->guard.store(1,memory_order_release);
	return 1;
}

int _pop(struct lf_queue_mpsc_t * queue, void ** dest){

	unsigned int tail = queue->tail.load(memory_order_relaxed);
	unsigned int old_head = queue->head.load(memory_order_relaxed);

	if(tail != old_head) { // not empty.

		struct pointer_guard * pg = &queue->list[tail%queue->cap];

		if(pg->guard.load(memory_order_acquire)){ // data is ready
			*dest = pg->ptr;
			pg->guard.store(0,memory_order_relaxed); // could be relaxed?
			queue->tail.store( tail + 1 , memory_order_release);
			return 1; // success? haha
		}
		//printf("data not ready. tail %d , head %d , cap%d\n",tail,old_head,queue->cap);
	}else{
		//printf("empty. ");
	}

	// A second ago it was empty or the data wasn't ready.
	return 0;
}

void init_lf_queue_spsc(struct lf_queue_mpsc_t * queue, unsigned int capacity){

	assert(capacity +1 != 0  && "capacity must be less Max unsigned int");

	queue->list = (struct pointer_guard*) malloc(sizeof (struct pointer_guard) * (capacity+1));
	queue->cap = capacity+1;
	queue->tail.store( 0, memory_order_relaxed);
	queue->head.store( 0, memory_order_relaxed);

	queue->pop = _pop;
	queue->push = _push;
}


#endif /* LF_QUEUE_MPSC_HPP_ */
