// lock-free-vs.cpp : main project file.

#include "stdafx.h"
#include <windows.h>
//#include "Queue_lf_s_mpsc.h"
#include "Queue_lf_ts_mpsc.h"
//#include "queue_lf_tc_mpsc.h"

static const int COUNT = 5000000;
static const int NUM_PROD = 2;
//typedef struct lf_queue_spsc_t QUEUE_TYPE;
//typedef struct queue_lf_s_mpsc_t QUEUE_TYPE;
typedef struct queue_lf_ts_mpsc_t<int> QUEUE_TYPE;
//typedef queue_lf_tc_mpsc<int> QUEUE_TYPE;


void func_prod(QUEUE_TYPE& queue, unsigned long long& returnValue){
	int j = 0;
	for (int i = 0; i < COUNT; i++) {

		while (!queue.push(&queue,i)){
		//while (!queue.push(i)){
			//if(j++ % 10000  == 0)printf("stuck %d, %d\n",j,i);
		}
		//if (i%10000 == 0)
			//printf("P%d\n",i);
		returnValue += i;
	}

	printf("prod %llu\n", returnValue);
}

void func_cons(QUEUE_TYPE& queue, unsigned long long& returnValue){
	
	//void* ptr;
	int ptr;

	for (int i = 0; i < COUNT *NUM_PROD; i++) {
		while (!queue.pop(&queue, &ptr)){
		//while (!queue.pop( std::ref(ptr))){
			//printf("pop failed, %d, %d\n",i,COUNT);
		}
		returnValue += (int)ptr;

	}
	printf("done %llu\n", returnValue);
}

int main(int argc, char* argv[]){

	unsigned int cap = 1000, i;
	unsigned long long prod_sum = 0, t;
	//boost::posix_time::ptime start, end;
	SYSTEMTIME start, end;
	//QUEUE_TYPE queue(cap);
	QUEUE_TYPE queue;
	queue.init_queue_lf_s_mpsc(&queue, cap);

	thread prods[NUM_PROD];
	unsigned long long prodRet[NUM_PROD];
	thread consumer;
	unsigned long long consRet;

	// INIT
	GetSystemTime(&start);

	// CREATE THREADS
	consRet = 0;
	consumer = thread(func_cons, 
					  std::ref(queue),
					  std::ref(consRet));

	for (i = 0; i<NUM_PROD; i++){
		prodRet[i] = 0;
		prods[i] = thread(func_prod, 
						  std::ref(queue),
						  std::ref(prodRet[i]));
	}

	// JOIN THREADS
	for (i = 0; i < NUM_PROD; i++){
		prods[i].join();
		prod_sum += prodRet[i];
	}
	consumer.join();

	// COMPUTE STATS
	GetSystemTime(&end);

	t = 1000 * (end.wSecond - start.wSecond)
		+ (end.wMilliseconds - start.wMilliseconds);

	double rate = COUNT*NUM_PROD / (t / 1000.0);

	printf("%f s,  c1=%llu\n", t/1000.0, consRet);
	printf("%f push/pops per second\n", rate);

	// CHECK FOR ERRORS
	if (prod_sum != consRet){
		printf("FAIL!!!!!!!!!!!!!\n");
		printf("prod_sum:%llu \n", prod_sum);
		printf("c1      :%llu \n", consRet);
	}
	while (1);
	return 0;
}
