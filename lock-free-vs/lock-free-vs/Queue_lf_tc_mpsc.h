/*
 * queue_lf_tc_mpsc.hpp
 *
 *  Created on: May 4, 2014
 *      Author: Peter Rindal
 *
 *      Bounded single producer single consumer
 *      non-blocking lock free queue.
 *
 *      g++ --std=c++11 ...
 */

#ifndef queue_lf_tc_mpsc_HPP_
#define queue_lf_tc_mpsc_HPP_
#include "stdafx.h"
#include "Guarded_data_tc.h"


using namespace std;

template <typename T> class queue_lf_tc_mpsc{
private:
	Guarded_data_tc<T>* list;
	unsigned int cap;
	unsigned int capMask;
	atomic<unsigned int> tail;
	atomic<unsigned int> head;

	int isPowerOfTwo(unsigned int);

public:
	queue_lf_tc_mpsc(unsigned int capacity){
		//assert(capacity +1 != 0  && "capacity must be less Max unsigned int");
		assert(isPowerOfTwo(capacity) && "capacity must be a power  oof two.");

		list = (Guarded_data_tc<T>*)malloc(sizeof(Guarded_data_tc<T>)*(capacity+1));
		cap = capacity;
		capMask = cap - 1;
		tail.store(0, memory_order_release);
		head.store(0, memory_order_release);
	}

	int push(T );
	int pop (T&);
};

template <typename T> int queue_lf_tc_mpsc<T>::isPowerOfTwo(unsigned int x)
{
	return ((x != 0) && !(x & (x - 1)));
}

template <typename T> int queue_lf_tc_mpsc<T>::push(T entry){

	unsigned int lcl_tail, lcl_head;

	do{
		lcl_tail = tail.load(memory_order_relaxed);
		lcl_head = head.load(memory_order_relaxed); 
		if (lcl_tail + cap <= lcl_head) // head - cap == tail
			return 0;
	// try and reserve head for our selves
	}while(! atomic_compare_exchange_weak_explicit(&head,
										  	  	   &lcl_head,
										  	  	   lcl_head + 1,
										  	  	   memory_order_release,
												   memory_order_relaxed));
	// the index of lcl_head is ours!
	// set the entry and then publish it to the world!
	//list[lcl_head % cap].publish(entry);
	list[lcl_head & capMask].publish(entry);

	return 1;
}

template <typename T> int queue_lf_tc_mpsc<T>::pop(T& dest){

	unsigned int lcl_tail = tail.load(memory_order_relaxed);
	unsigned int lcl_head = head.load(memory_order_relaxed);

	if(lcl_tail != lcl_head) {					// not empty.
		//Guarded_data_tc<T>* gd = &list[lcl_tail % cap];
		Guarded_data_tc<T>* gd = &list[lcl_tail & capMask];
		if(gd->tryGet(dest)){					// try and get the data
			gd->rescind();								// mark the data as no longer valid
			tail.store( lcl_tail + 1 , memory_order_release);
			return 1;									// success? haha
		}	
		//printf("data not ready. tail %d , head %d , cap%d\n",lcl_tail,lcl_head, cap);
	}
	// A second ago it was empty or the data wasn't ready.
	return 0;
}
#endif /* queue_lf_tc_mpsc_HPP_ */
