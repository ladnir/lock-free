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
#include <sys/time.h>

//#include "lf_queue_spsc.hpp"
#include "l_queue_mpmc.hpp"

#define COUNT 10000000
//typedef struct lf_queue_spsc_t QUEUE_TYPE;
typedef struct l_queue_mpmc_t QUEUE_TYPE;

void * func_prod(void *arg){
	QUEUE_TYPE* queue = (QUEUE_TYPE*) arg;
	int i;
	unsigned long long  *s = (unsigned long long*)malloc(sizeof(unsigned long long));
	*s = 0;
	for(i = 0; i < COUNT; i++) {
		//printf("push %d\n",i +100);
		if(! queue->push(queue,(void*) (i+100) )){
			i--;
			continue;
		}
		*s = *s + (i+100);
	}

	//printf("prod %d\n",s);
	return s;
}

void * func_cons(void *arg){
	QUEUE_TYPE* queue = (QUEUE_TYPE*) arg;

	int i;
	unsigned long long  *s = (unsigned long long*)malloc(sizeof(unsigned long long));
	*s = 0;

	for(i =0; i < COUNT; i++) {
		//printf("p\n");
		void * p = queue->pop(queue);
		if(!p){
			i--;
			continue;
		}
		*s = *s + (int)p;
		//printf("pop %d\n", (int)p);
	}
	//printf("done %d\n",s);
	return s;
}

int main(int argc, char* argv[]){

	int cap = 1000;
	QUEUE_TYPE queue;

	init_lf_queue_spsc(&queue, cap);
	pthread_t producer, consumer;

	struct timeval start,end;
	gettimeofday(&start,NULL);

	pthread_create(&producer,0,&func_prod,&queue);
	pthread_create(&consumer,0,&func_cons,&queue);
	void * r1,*r2;

	pthread_join(producer,&r1);
	pthread_join(consumer,&r2);

	gettimeofday(&end,NULL);

	unsigned long long t = 1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000;
	printf("%llu ms\n", t);

	double rate = COUNT / (t*1.0/1000);
	printf("%f push/pops per second\n",rate);

	if(*(unsigned long long*)r1 != *(unsigned long long*)r2){
		printf("FAIL!!!!!!!!!!!!!\n%d \n%d\n",(unsigned long long )r1,(unsigned long long )r2);
	}

	return 0;
}
