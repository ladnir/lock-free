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


using namespace std;

struct lf_queue_mpsc_t{

	void ** list;
	int cap;
	atomic<int> barrier; 	// just for correctness,
	atomic<int> tail; 	// ints are normally atomic...
	atomic<int> head; // temp head for producers.

	int (*push)(struct lf_queue_mpsc_t *, void *);
	void *(*pop)(struct lf_queue_mpsc_t *);
};

int _push(struct lf_queue_mpsc_t * queue, void * entry){
	//sleep(1);
	//printf("push\n");
restart:

	int tail = queue->tail.load(memory_order_relaxed);
	int old_head = queue->head.load(memory_order_relaxed);

	if( tail== (old_head+1) % queue->cap )return 0;

	// try and reserve head for our selves
	if(! atomic_compare_exchange_weak(&queue->head, &old_head, old_head+1) )
		goto restart;
	// the index of head is ours!
	//printf("got it\n");
	queue->list[old_head] = entry; //Store
	atomic_thread_fence(memory_order_release); //StoreStore
	queue->head.store((old_head+1)%queue->cap, memory_order_relaxed); //Store

	// wait for barrier to catch up and the increment it
	int expecting = old_head ;
	while(! atomic_compare_exchange_weak(&queue->barrier, &expecting, (old_head + 1) % queue->cap) ){
		expecting = old_head ;
		//printf("barrier failed\n");
	}
	//printf("done\n");
	return 1;
}

void * _pop(struct lf_queue_mpsc_t * queue){
	//sleep(1);
	//printf("pop\n");

	int tail = queue->tail.load(memory_order_relaxed);
	int barrier = queue->barrier.load(memory_order_relaxed);

	if(tail == barrier) return NULL;

	void * entry = queue->list[tail]; //Load
	atomic_thread_fence(memory_order_acquire); // LoadStore
	queue->tail.store( (tail + 1 ) %queue->cap, memory_order_relaxed); //Store

	return entry;
}

void init_lf_queue_spsc(struct lf_queue_mpsc_t * queue, int capacity){

	queue->list = (void **) malloc(sizeof (void*) * capacity+1);
	queue->cap = capacity+1;
	queue->barrier.store( 0, memory_order_relaxed);
	queue->tail.store( 0, memory_order_relaxed);
	queue->head.store( 0, memory_order_relaxed);

	queue->pop = _pop;
	queue->push = _push;
}


#endif /* LF_QUEUE_MPSC_HPP_ */
