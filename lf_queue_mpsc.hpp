/*
 * lf_queue_mpsc.hpp
 *
 *  Created on: May 4, 2014
 *      Author: Peter Rindal
 *
 *      Bounded single producer single consumer
 *      non-blocking lock free queue.
 *
 *      g++ --std=c++11 ...
 */

#ifndef LF_QUEUE_MPSC_HPP_
#define LF_QUEUE_MPSC_HPP_
#include <stdlib.h>
#include <atomic>
#include <thread>
#include <cassert>


using namespace std;

template <class T> class Guarded_data{
public:
		Guarded_data(){guard = 0;}
	 	void inline publish(T data);
	 	int inline tryGet(T* dest);
	 	void inline rescind();
private:
		T data;
		atomic<unsigned int> guard;
};

template <class T> void inline Guarded_data<T>::publish(T entry){
	data = entry;
	guard.store(1,memory_order_release);
}
template <class T> int inline Guarded_data<T>::tryGet(T* dest){

	if(guard.load(memory_order_acquire)){
		*dest = data;
		return 1;
	}
	return 0;
}
template <class T> void inline Guarded_data<T>::rescind(){
	guard.store(0,memory_order_release);
}

template <class T> class Lf_queue_mpsc{
public:
	Lf_queue_mpsc(unsigned int capacity){
		assert(capacity +1 != 0  && "capacity must be less Max unsigned int");

		list = (Guarded_data*) malloc(sizeof (Guarded_data<T>) * (capacity+1));
		cap = capacity+1;
		tail.store( 0, memory_order_relaxed);
		head.store( 0, memory_order_relaxed);
	}

	int push(T );
	int pop (T*);
private:
	Guarded_data<T>* list;
	unsigned int cap;
	atomic<unsigned int> tail;
	atomic<unsigned int> head;
};

template <class T> int Lf_queue_mpsc<T>::push( T entry){

	unsigned int lcl_tail, lcl_head;

	do{
		lcl_tail = this->tail.load(memory_order_relaxed);
		lcl_head = this->head.load(memory_order_relaxed);
		if( tail + cap -1 <= lcl_head )return 0;

	// try and reserve head for our selves
	}while(! atomic_compare_exchange_weak_explicit(&this->head,
										  	  	   &lcl_head,
										  	  	   lcl_head+1,
										  	  	   memory_order_release,
										  	  	   memory_order_relaxed ));
	// the index of lcl_head is ours!
	// set the entry and then publish it
	list[lcl_head % cap].publish(entry);
	return 1;
}

template <class T> int Lf_queue_mpsc<T>::pop(T* dest){

	unsigned int lcl_tail = tail.load(memory_order_relaxed);
	unsigned int lcl_head = head.load(memory_order_relaxed);

	if(lcl_tail != lcl_tail) { // not empty.
		Guarded_data<T>* gd = &list[lcl_tail % cap];
		if(gd->tryGet(dest)){ // try and get the data
			gd->rescind(); // mark the data as no longer valid
			tail.store( lcl_tail + 1 , memory_order_release);
			return 1; // success? haha
		}
		//printf("data not ready. tail %d , head %d , cap%d\n",tail,old_head,queue->cap);
	}

	// A second ago it was empty or the data wasn't ready.
	return 0;
}


#endif /* LF_QUEUE_MPSC_HPP_ */
