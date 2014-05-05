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


//#include "lf_queue_spsc.hpp"
//#include "l_queue_mpmc.hpp"
#include "lf_queue_mpsc.hpp"

#define COUNT 10000000
//typedef struct lf_queue_spsc_t QUEUE_TYPE;
//typedef struct l_queue_mpmc_t QUEUE_TYPE;
typedef struct lf_queue_mpsc_t QUEUE_TYPE;


void * func_prod(void *arg,int id){
	QUEUE_TYPE* queue = (QUEUE_TYPE*) arg;
	unsigned long long i,c=0;
	unsigned long long  *s = (unsigned long long*)malloc(sizeof(unsigned long long));
	*s = 0;
	for(i = 0; i < COUNT; i++) {
		//
		if(! queue->push(queue,(void*) (i+100) )){
			i--;
//			c++;
//			if(c>100){
//				int head = queue->head.load(memory_order_relaxed);
//				int tail = queue->tail.load(memory_order_relaxed);
//				int flag = queue->flags[tail%queue->cap];
//				printf("push%d beep %llu\nh=%d \nt=%d   , %d\n", id,i,head,tail,flag);
//				c=0;
//			}
			//printf("f\n");
			continue;
		}
//		c=0;
		*s = *s + (i+100);
		//printf("pushed %d\n",i +100);
	}

	//printf("prod %d\n",s);
	return s;
}
void * func_prod2(void *arg){
	return func_prod(arg,2);
}

void * func_prod1(void *arg){
	return func_prod(arg,1);
}
void * func_cons(void *arg){
	QUEUE_TYPE* queue = (QUEUE_TYPE*) arg;

	unsigned long long i,c=0;
	unsigned long long  *s = (unsigned long long*)malloc(sizeof(unsigned long long));
	*s = 0;

	for(i =0; i < COUNT *2;) {
		//printf("p\n");
		void * p = queue->pop(queue);
		if(!p){

//			c++;
//			if(c>100){
//				int head = queue->head.load(memory_order_relaxed);
//				int tail = queue->tail.load(memory_order_relaxed);
//				int flag = queue->flags[tail%queue->cap].load(memory_order_relaxed);
//				printf("pop beep %llu\nh=%d \nt=%d   , %d===============================================\n", i,head,tail,flag);
//				c=0;
//			}
			continue;
		}
		i++;
//		c=0;
		*s = *s + (int)p;
		//printf("popped %d\n", (int)p);
	}
	//printf("done %d\n",s);
	return s;
}

int main(int argc, char* argv[]){

	int cap = 1000;
	QUEUE_TYPE queue;

	init_lf_queue_spsc(&queue, cap);
	pthread_t producer1, producer2, consumer;

	struct timeval start,end;
	gettimeofday(&start,NULL);

	pthread_create(&producer1,0,&func_prod1,&queue);
	pthread_create(&producer2,0,&func_prod2,&queue);
	pthread_create(&consumer,0,&func_cons,&queue);
	void * p1,*p2, *c1;

	pthread_join(producer1,&p1);
	pthread_join(producer2,&p2);
	pthread_join(consumer,&c1);

	gettimeofday(&end,NULL);

	unsigned long long t = 1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000;
	printf("%llu ms,  c1=%llu\n", t,*(unsigned long long *)c1);

	double rate = COUNT*2.0/t / (1.0/1000.0);
	printf("%f push/pops per second\n",rate);

	if(*(unsigned long long*)p1 + *(unsigned long long*)p2 != *(unsigned long long*)c1){
		printf("FAIL!!!!!!!!!!!!!\np1:%llu \np2:%llu\nc1:%llu\n",*(unsigned long long* )p1,*(unsigned long long *)p2,*(unsigned long long *)c1);
	}

	return 0;
}
