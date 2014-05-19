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

template<typename T>
struct data_guard_ts{

	T data;
	atomic<unsigned int> guard;
};

template<typename T>
struct queue_lf_ts_mpsc_t{

	struct data_guard_ts<T>* list;
	unsigned int cap;

	atomic<unsigned int> tail;
	atomic<unsigned int> head;
	
	void init(struct queue_lf_ts_mpsc_t<T> * queue, unsigned int capacity){

		assert(capacity + 1 != 0 && "capacity must be less Max unsigned int");

		queue->list = (struct data_guard_ts<T>*) malloc(sizeof (struct data_guard_ts<T>) * (capacity + 1));
		queue->cap = capacity + 1;
		queue->tail.store(0, memory_order_relaxed);
		queue->head.store(0, memory_order_relaxed);
	}

	template<typename T>
	int push(struct queue_lf_ts_mpsc_t<T> * queue, T entry){

		unsigned int lcl_tail, lcl_head;

		do{
			lcl_tail = queue->tail.load(memory_order_relaxed);
			lcl_head = queue->head.load(memory_order_relaxed);
			if (lcl_tail + queue->cap - 1 <= lcl_head)
				return 0;
			// try and reserve head for our selves
		} while (!atomic_compare_exchange_weak_explicit(&queue->head,
														&lcl_head,
														lcl_head + 1,
														memory_order_release,
														memory_order_relaxed));
		// the index of old_head is ours!

		struct data_guard_ts<T> * dg = &queue->list[lcl_head%queue->cap];
		dg->data = entry;
		dg->guard.store(1, memory_order_release);
		return 1;
	}

	template<typename T>
	int pop(struct queue_lf_ts_mpsc_t<T> * queue, T& dest){

		unsigned int lcl_tail = queue->tail.load(memory_order_relaxed);
		unsigned int lcl_head = queue->head.load(memory_order_relaxed);
	
		if (lcl_tail != lcl_head) { // not empty.

			struct data_guard_ts<T> * dg = &queue->list[lcl_tail%queue->cap];

			if (dg->guard.load(memory_order_acquire)){ // data is ready
				dest = dg->data;
				dg->guard.store(0, memory_order_relaxed); // could be relaxed?
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

};
#endif /* QUEUE_LF_S_MPSC_H_ */