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
#include <atomic>
#include <thread>
using namespace std;


#include "lf_queue_spsc.hpp"
#include "l_queue_mpmc.hpp"
#include "lf_queue_mpsc.hpp"

static const int COUNT = 10000000;
//typedef struct lf_queue_spsc_t QUEUE_TYPE;
typedef struct l_queue_mpmc_t QUEUE_TYPE;
//typedef struct lf_queue_mpsc_t QUEUE_TYPE;


void * func_prod(void *arg){
	QUEUE_TYPE* queue = (QUEUE_TYPE*) arg;

	int i;
	unsigned long long  *s = (unsigned long long*)malloc(sizeof(unsigned long long));
	*s = 0;

	for(i = 0; i < COUNT;i++ ) {

		int * ptr = (int*)malloc(sizeof(int*));
		*ptr = i;
		//sleep(1);
		//printf("push\n");
		while(! queue->push(queue,(void*) ptr ));

		*s += i;
	}

	printf("prod %llu\n",*s);
	return s;
}

void * func_cons(void *arg){
	QUEUE_TYPE* queue = (QUEUE_TYPE*) arg;

	int i;
	unsigned long long  *s = (unsigned long long*)malloc(sizeof(unsigned long long));
	*s = 0;

	for(i =0; i < COUNT *2;i++) {
		int* ptr;

		//sleep(1);
		//printf("pop\n");
		while(! queue->pop(queue,(void**)&ptr)){
			//printf("pop failed, %d, %d\n",i,COUNT);
		}

		*s += *ptr;
		//free(ptr);
	}
	printf("done %llu\n",*s);
	return s;
}

int main(int argc, char* argv[]){

	int cap = 10000;
	QUEUE_TYPE queue;

	init_lf_queue_spsc(&queue, cap);
	pthread_t producer1, producer2, consumer;

	struct timeval start,end;
	gettimeofday(&start,NULL);

	pthread_create(&producer1,0,&func_prod,&queue);
	pthread_create(&producer2,0,&func_prod,&queue);
	pthread_create(&consumer,0,&func_cons,&queue);
	unsigned long long * p1,*p2, *c1;

	pthread_join(producer1,(void**) &p1);
	pthread_join(producer2,(void**) &p2);
	pthread_join(consumer ,(void**) &c1);

	gettimeofday(&end,NULL);

	unsigned long long t;

	t = 1000 * (end.tv_sec - start.tv_sec)
	  + (end.tv_usec - start.tv_usec) / 1000;

	printf("%llu ms,  c1=%llu\n", t,*c1);

	double rate = COUNT*2.0/(t/1000.0);
	printf("%f push/pops per second\n",rate);

	if(*p1 + *p2 != *c1){
		printf("FAIL!!!!!!!!!!!!!\n");
		printf("p1:%llu \n",*p1);
		printf("p2:%llu\n",*p2);
		printf("c1:%llu\n",*c1);
	}

	return 0;
}
