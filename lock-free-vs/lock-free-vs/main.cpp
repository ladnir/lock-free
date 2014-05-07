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
static const int NUM_PROD = 6;
//typedef struct lf_queue_spsc_t QUEUE_TYPE;
//typedef struct l_queue_mpmc_t QUEUE_TYPE;
typedef Lf_queue_mpsc<int*> QUEUE_TYPE;


void * func_prod(void *arg){
	QUEUE_TYPE* queue = (QUEUE_TYPE*) arg;

	int i;
	unsigned long long  *s = (unsigned long long*)malloc(sizeof(unsigned long long));
	*s = 0;
	int * ptr = (int*)malloc(sizeof(int*));
	for(i = 0; i < COUNT;i++ ) {

		//int * ptr = (int*)malloc(sizeof(int*));
		//*ptr = i;
		while(! queue->push( ptr ));

		//*s += i;
	}

	printf("prod %llu\n",*s);
	return s;
}

void * func_cons(void *arg){
	QUEUE_TYPE* queue = (QUEUE_TYPE*) arg;

	int i;
	unsigned long long  *s = (unsigned long long*)malloc(sizeof(unsigned long long));
	*s = 0;

	for(i =0; i < COUNT *NUM_PROD;i++) {
		int* ptr;

		//sleep(1);
		//printf("pop\n");
		while(! queue->pop(&ptr)){
			//printf("pop failed, %d, %d\n",i,COUNT);
		}

		//*s += *ptr;
		//free(ptr);
	}
	printf("done %llu\n",*s);
	return s;
}

int main(int argc, char* argv[]){

	unsigned int cap = 10000,i;
	unsigned long long * p1, *c1,prod_sum=0, t;
	struct timeval start,end;
	QUEUE_TYPE queue;
	pthread_t prods[NUM_PROD];
	pthread_t consumer;

	// INIT
	//init_lf_queue_spsc(&queue, cap);
	queue = new QUEUE_TYPE(cap);
	gettimeofday(&start,NULL);

	// CREATE THREADS
	pthread_create(&consumer,0,&func_cons,&queue);
	for(i=0;i<NUM_PROD;i++){
		pthread_create(&prods[i],0,&func_prod,&queue);
	}

	// JOIN THREADS
	for(i=0;i<NUM_PROD;i++){
		pthread_join(prods[i],(void**) &p1);
		prod_sum += *p1;
	}
	pthread_join(consumer ,(void**) &c1);

	// COMPUTE STATS
	gettimeofday(&end,NULL);
	t = 1000 * (end.tv_sec - start.tv_sec)
	  + (end.tv_usec - start.tv_usec) / 1000;
	double rate = COUNT*NUM_PROD/(t/1000.0);

	printf("%llu ms,  c1=%llu\n", t,*c1);
	printf("%f push/pops per second\n",rate);

	// CHECK FOR ERRORS
	if(prod_sum!= *c1){
		printf("FAIL!!!!!!!!!!!!!\n");
		printf("prod_sum:%llu \n",prod_sum);
		printf("c1:%llu\n",*c1);
	}

	return 0;
}
