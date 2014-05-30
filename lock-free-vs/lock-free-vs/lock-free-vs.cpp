// lock-free-vs.cpp : main project file.

#include "stdafx.h"
#include <windows.h>
//#include "Queue_lf_s_mpsc.h"
//#include "Queue_lf_ts_mpsc.h"
#include "queue_lf_tc_mpsc.h"
#include "Queue_lf_spmc.cpp"
static const unsigned COUNT = 1000000; // 65537
static const int NUM_PROD = 1;
static const int NUM_CONS = 4;
//typedef struct lf_queue_spsc_t QUEUE_TYPE;
//typedef struct queue_lf_s_mpsc_t QUEUE_TYPE;
//typedef struct queue_lf_ts_mpsc_t<int> QUEUE_TYPE;
//typedef queue_lf_tc_mpsc<int> QUEUE_TYPE;
typedef Queue_lf_spmc<int> QUEUE_TYPE;


void func_prod(QUEUE_TYPE& queue, unsigned long long& returnValue){
	int j = 0;
	
	for (int i = 0; i < COUNT; i++) {

		//while (!queue.push(&queue,i)){
		while (!queue.push(i)){
			//if(j++ % 10000  == 0)printf("stuck %d, %d\n",j,i);
		}
	//	if (i%10000 == 0)
			//printf("P%d\n",i);
		returnValue += i;
	}

	printf("prod %llu\n", returnValue);
}

void func_cons(QUEUE_TYPE& queue, unsigned long long& returnValue){
	
	//void* ptr;
	int ptr, loopCount = COUNT *NUM_PROD / NUM_CONS;
	returnValue = 0;
	
	for (int i = 0; i < loopCount; i++) {
		//while (!queue.pop(&queue, ptr)){
		while (!queue.pop( ptr)){
			//printf("pop failed, %d, %d\n",i,COUNT);
		}
		//if (!(i % 10000))printf("cons %d\n",i);
		//if (ptr > COUNT) printf("oops\n");
		returnValue += ptr;
		//printf("%d, %d, %llu\n", ptr, sum, returnValue);
	}
	
	printf("cons %llu\n", returnValue);
}

int main(int argc, char* argv[]){

	unsigned int cap = 64, i;
	unsigned long long prod_sum = 0, cons_sum = 0;
	double t;
	//boost::posix_time::ptime start, end;
	SYSTEMTIME start, end;
	QUEUE_TYPE queue(cap);
	//QUEUE_TYPE queue;
	//queue.init(&queue, cap);

	thread prods[NUM_PROD];
	unsigned long long prodRet[NUM_PROD];
	thread consumer[NUM_CONS];
	unsigned long long consRet[NUM_CONS];

	// INIT
	GetSystemTime(&start);

	// CREATE THREADS
	for (i = 0; i < NUM_CONS; i++){
		consRet[i] = 0;
		consumer[i] = thread(func_cons,
						 	 std::ref(queue),
							 std::ref(consRet[i]));
	}


	for (i = 0; i<NUM_PROD; i++){
		prodRet[i] = 0;
		prods[i] = thread(func_prod, 
						  std::ref(queue),
						  std::ref(prodRet[i]));
	}

	// JOIN THREADS
	for (i = 0; i < NUM_CONS; i++){
		consumer[i].join();
		cons_sum += consRet[i];
	}
	for (i = 0; i < NUM_PROD; i++){
		prods[i].join();
		prod_sum += prodRet[i];
	}
	

	// COMPUTE STATS
	GetSystemTime(&end);

	t = 60.0 * 60.0 * (end.wHour - start.wHour)
		+ 60.0 * (end.wMinute -  start.wMinute)
		+ (end.wSecond * 1.0 - start.wSecond)
		+ ( (end.wMilliseconds * 1.0- start.wMilliseconds)/1000.0);

	double rate = COUNT*NUM_PROD / (t );

	printf("time:%f s\n#c: %d\n#p: %d\nCOUNT: %d\n", t, NUM_CONS,NUM_PROD,COUNT);
	printf("%f push/pops per second\n", rate);

	// CHECK FOR ERRORS
	if (prod_sum != cons_sum){
		printf("FAIL!!!!!!!!!!!!!\n");
		printf("prod_sum:%llu \n", prod_sum);
		printf("cons_sum:%llu \n", cons_sum);
	}
	while (1);
	return 0;
}
