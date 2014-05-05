/*
 * queue_spsc.h
 *
 *  Created on: May 4, 2014
 *      Author: Peter Rindal
 *
 *      Bounded single producer single consumer
 *      non-blocking lock free queue.
 *
 *      g++ --std=c++11 -lpthread ...
 */
#include <stdlib.h>

#ifndef QUEUE_SPSC_H_
#define QUEUE_SPSC_H_
#include <atomic>
#include <thread>


using namespace std;

struct lf_queue_spsc_t{

	void ** list;
	int cap;
	atomic<int> head; // just for correctness,
	atomic<int> tail; // ints are normally atomic...

	int (*push)(struct lf_queue_spsc_t *, void *);
	void *(*pop)(struct lf_queue_spsc_t *);
};

int _push(struct lf_queue_spsc_t * queue, void * entry){

	int tail = queue->tail.load(memory_order_relaxed);
	int head = queue->head.load(memory_order_relaxed);

	if( tail== (head + 1) % queue->cap )return 0;

	queue->list[head] = entry; //Store
	atomic_thread_fence(memory_order_release); //StoreStore
	queue->head.store((head+1) % queue->cap ,memory_order_relaxed); //Store

	return 1;
}

void * _pop(struct lf_queue_spsc_t * queue){
	int tail = queue->tail.load(memory_order_relaxed);
	int head = queue->head.load(memory_order_relaxed);

	if(tail == head) return NULL;

	void * entry = queue->list[tail]; //Load
	atomic_thread_fence(memory_order_acquire); // LoadStore
	queue->tail.store( (tail + 1 ) %queue->cap, memory_order_relaxed); //Store

	return entry;
}

void init_lf_queue_spsc(struct lf_queue_spsc_t * queue, int capacity){

	queue->list = (void **) malloc(sizeof (void*) * capacity+1);
	queue->cap = capacity+1;
	queue->head.store( 0, memory_order_relaxed);
	queue->tail.store( 0, memory_order_relaxed);

	queue->pop = _pop;
	queue->push = _push;
}
#endif /* QUEUE_SPSC_H_ */
