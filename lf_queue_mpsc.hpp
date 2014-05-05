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

#define EMPTY 0
#define FULL 1
#define PARTIAL 2

using namespace std;

struct lf_queue_mpsc_t{

	atomic<int> * flags;
	void ** list;
	int cap;

	atomic<int> tail; 	// ints are normally atomic...
	atomic<int> head; // temp head for producers.

	int (*push)(struct lf_queue_mpsc_t *, void *);
	void *(*pop)(struct lf_queue_mpsc_t *);
};

int _push(struct lf_queue_mpsc_t * queue, void * entry){
	//sleep(1);
	//printf("push\n");
	int r =0;
restart:

	int tail = queue->tail.load(memory_order_acquire);
	int old_head = queue->head.load(memory_order_relaxed);

	if( tail <= (old_head+1) - queue->cap )return 0;

	// try and reserve head for our selves
	if(! atomic_compare_exchange_weak(&queue->head, &old_head, old_head+1 ) ){
		goto restart;
	}

	// the index of old_head is ours!
	queue->list[old_head%queue->cap] = entry;  //Store
	atomic_thread_fence(memory_order_release); //StoreStore
	queue->flags[old_head%queue->cap].store( FULL,memory_order_relaxed);  //Store

	//printf("done\n");
	return 1;
}

void * _pop(struct lf_queue_mpsc_t * queue){
	//sleep(1);
	//printf("pop\n");

	int tail = queue->tail.load(memory_order_relaxed);
	int head = queue->head.load(memory_order_relaxed);

	if(tail == head) return NULL;

	while(queue->flags[tail%queue->cap].load(memory_order_relaxed) != FULL); // spin until write has completed

	void * entry = queue->list[tail%queue->cap]; //Load
	queue->flags[tail%queue->cap].store( EMPTY,memory_order_relaxed);       //Store
	atomic_thread_fence(memory_order_release); // LoadStore , StoreStore
	queue->tail.store( tail + 1 , memory_order_relaxed); //Store

	return entry;
}

void init_lf_queue_spsc(struct lf_queue_mpsc_t * queue, int capacity){

	queue->flags = (atomic<int>*) malloc(sizeof(atomic<int>)* (capacity+1));
	queue->list = (void **) malloc(sizeof (void*) * (capacity+1));
	queue->cap = capacity+1;
	queue->tail.store( 0, memory_order_relaxed);
	queue->head.store( 0, memory_order_relaxed);

	queue->pop = _pop;
	queue->push = _push;
}


#endif /* LF_QUEUE_MPSC_HPP_ */
