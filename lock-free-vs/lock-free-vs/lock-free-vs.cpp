// lock-free-vs.cpp : main project file.

#include "stdafx.h"
#include <windows.h>

static const int COUNT = 10000000;
static const int NUM_PROD = 6;
//typedef struct lf_queue_spsc_t QUEUE_TYPE;
//typedef struct l_queue_mpmc_t QUEUE_TYPE;
typedef Queue_lf_mpsc<int> QUEUE_TYPE;


void func_prod(QUEUE_TYPE& queue, int& returnValue){

	for (int i = 0; i < COUNT; i++) {
		while (!queue.push(i));
		//returnVlaue += i;
	}

	printf("prod %llu\n", returnValue);
}

void func_cons(QUEUE_TYPE& queue, int& returnValue){
	
	int ptr;
	for (int i = 0; i < COUNT *NUM_PROD; i++) {

		while (!queue.pop( std::ref(ptr))){
			//printf("pop failed, %d, %d\n",i,COUNT);
		}
		//returnValue += ptr;

	}
	printf("done %llu\n", returnValue);
}

int main(int argc, char* argv[]){

	unsigned int cap = 10000, i;
	unsigned long long prod_sum = 0, t;
	//boost::posix_time::ptime start, end;
	SYSTEMTIME start, end;
	QUEUE_TYPE queue(cap);

	thread prods[NUM_PROD];
	int prodRet[NUM_PROD];
	thread consumer;
	int consRet;

	// INIT
	GetSystemTime(&start);

	// CREATE THREADS
	consumer = thread(func_cons, 
					  std::ref(queue),
					  std::ref(consRet));

	for (i = 0; i<NUM_PROD; i++){
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

	printf("%llu ms,  c1=%llu\n", t, consRet);
	printf("%f push/pops per second\n", rate);

	// CHECK FOR ERRORS
	if (prod_sum != consRet){
		printf("FAIL!!!!!!!!!!!!!\n");
		printf("prod_sum:%llu \n", prod_sum);
		printf("c1:%llu\n", consRet);
	}

	return 0;
}
