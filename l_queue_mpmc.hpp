/*
 * queue_spsc.h
 *
 *  Created on: May 4, 2014
 *      Author: Peter Rindal
 *
 *      Bounded multi producer multi consumer
 *      non-blocking LOCKING queue.
 *
 *      g++ --std=c++11 -lpthread ...
 */

#ifndef L_QUEUE_MPMC_H_
#define L_QUEUE_MPMC_H_

#include <stdlib.h>
#include <semaphore.h>


using namespace std;

struct l_queue_mpmc_t{

	void ** list;
	int cap;
	int head;
	int tail;

	sem_t mutex;

	int (*push)(struct l_queue_mpmc_t *, void *);
	void *(*pop)(struct l_queue_mpmc_t *);
};

int _push(struct l_queue_mpmc_t * queue, void * entry){

	sem_wait(&queue->mutex);
	int tail = queue->tail;//.load(memory_order_relaxed);
	int head = queue->head;//.load(memory_order_relaxed);

	if( tail== (head + 1) % queue->cap ){
		sem_post(&queue->mutex);
		return 0;
	}

	queue->list[head] = entry;
	queue->head =(head+1) % queue->cap;

	sem_post(&queue->mutex);
	return 1;
}

void * _pop(struct l_queue_mpmc_t * queue){

	sem_wait(&queue->mutex);
	int tail = queue->tail;//.load(memory_order_relaxed);
	int head = queue->head;//.load(memory_order_relaxed);

	if(tail == head){
		sem_post(&queue->mutex);
		return NULL;
	}

	void * entry = queue->list[tail];
	queue->tail= (tail + 1 ) %queue->cap;//.store( , memory_order_relaxed);

	sem_post(&queue->mutex);
	return entry;
}

void init_lf_queue_spsc(struct l_queue_mpmc_t * queue, int capacity){

	queue->list = (void **) malloc(sizeof (void*) * capacity+1);
	queue->cap = capacity+1;
	queue->head = 0;//.store( 0, memory_order_relaxed);
	queue->tail = 0;//.store( 0, memory_order_relaxed);

	sem_init(&queue->mutex,0, 1);
	queue->pop = _pop;
	queue->push = _push;
}
#endif /* L_QUEUE_MPMC_H_ */
