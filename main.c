/*
 * main.c
 *
 *  Created on: May 4, 2014
 *      Author: rindalp
 */
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "lf_queue_spsc.h"

#define COUNT 10
typedef struct lf_queue_spsc_t QUEUE_TYPE;

void * func_prod(void *arg){
	QUEUE_TYPE* queue = (QUEUE_TYPE*) arg;
	int i;

	for(i = 0; i < COUNT; i++) {
		printf("push %d\n",i +100);
		if(! queue->push(queue,(void*) i+100)) i--;
	}

	return NULL;
}

void * func_cons(void *arg){
	QUEUE_TYPE* queue = (QUEUE_TYPE*) arg;

	int i;

	for(i =0; i < COUNT; i++) {
		//printf("p\n");
		void * p = queue->pop(queue);
		if(!p){
			i--;
			//continue;
		}
		printf("pop %d\n", (int)p);
	}
	printf("done\n");
	return NULL;
}

void main(int argc, char* argv[]){

	int cap = 3;
	QUEUE_TYPE queue;

	printf("in\n");
	init_lf_queue_spsc(&queue, cap);
	printf("init\n");
	pthread_t producer, consumer;

	pthread_create(&producer,0,&func_prod,&queue);
	pthread_create(&consumer,0,&func_cons,&queue);
	void * r;

	pthread_join(producer,&r);
	pthread_join(consumer,&r);


}
