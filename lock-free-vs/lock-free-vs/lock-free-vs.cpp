// lock-free-vs.cpp : main project file.

#include "stdafx.h"
#include <windows.h>
//#include "queue_lf_tc_mpsc.h"
//#include "Queue_lf_spmc.cpp"
#include "Deque_lf_tmc.cpp"
static const unsigned COUNT = 10; // 65537
static const int NUM_PROD = 1;
static const int NUM_CONS = 1;
//typedef queue_lf_tc_mpsc<int> QUEUE_TYPE;
//typedef Queue_lf_spmc<int> QUEUE_TYPE;
typedef Deque_lf_tmc<int> QUEUE_TYPE;

void func_prod(QUEUE_TYPE& queue, unsigned long long& returnValue, unsigned long long& popRet){
	int k,j = 0;
	int notPopped = 0;
	for (int i = 0; i < COUNT * 3; i++) {
		j = 0;
		k = i % 3;
		//while (!queue.push(i)){
		if (k == 0 || k == 1){
			while (!queue.pushHead(i)){
				if (j++ % 1000 == 0)printf("stuck head push %d, %d\n", j, i);
			}
			returnValue += i;
		}else{
			if(!queue.popHead(k)){
				notPopped++;
			}
			else{
				popRet += k;
			}
		}
	//	if (i%10000 == 0)
			//printf("P%d\n",i);
	}
	for (int i = 0; i < notPopped; i++){

		while (!queue.popHead(k)){
			if (j++ % 1 == 0)printf("stuck head pop %d, %d\n", j, i);
		}
		popRet += k;
	}
	printf("prodpop %llu\n", popRet);
	printf("prod %llu\n", returnValue);
}

void func_cons(QUEUE_TYPE& queue, unsigned long long& returnValue){
	
	//void* ptr;
	int ptr, loopCount = COUNT *NUM_PROD / NUM_CONS;
	returnValue = 0;
	
	for (int i = 0; i < loopCount; i++) {
		//while (!queue.pop(ptr)){
		while (!queue.popTail(ptr)){
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
	unsigned long long prodPopped = 0;
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
						  std::ref(prodRet[i]),
						  std::ref(prodPopped));
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

	double rate = 2* COUNT*NUM_PROD / (t );

	printf("time:%f s\n#c: %d\n#p: %d\nCOUNT: %d\n", t, NUM_CONS,NUM_PROD,COUNT);
	printf("%f push/pops per second\n", rate);

	// CHECK FOR ERRORS
	if (prod_sum != cons_sum + prodPopped){
		printf("FAIL!!!!!!!!!!!!!\n");
		printf("prod_sum:%llu \n", prod_sum);
		printf("prod_pop:%llu \n", prodPopped);
		printf("cons_sum:%llu \n", cons_sum);
	}
	while (1);
	return 0;
}
