/*
 * queue_spsc.h
 *
 *  Created on: May 4, 2014
 *      Author: Peter Rindal
 *
 *      Bounded multi producer multi consumer
 *      non-blocking LOCKING queue.
 *
 *      g++ --std=c++11 -lpthread ...
 */

#ifndef L_QUEUE_MPMC_H_
#define L_QUEUE_MPMC_H_

#include "stdafx.h"
#include <mutex>

using namespace std;

template <class T>
class Queue_l_mpmc{
public:
	Queue_l_mpmc(unsigned int capacity){
		cap = capacity;
		list.reserve(cap);
		head = 0;
		tail = 0;
	}

	int push(T);
	int pop(T&);
private:
	vector<T> list;
	int cap;
	int head;
	int tail;
	mutex mtx;
};

template<class T> int Queue_l_mpmc<T>::push(T entry){
	mtx.lock();

	if( tail== (head + 1) % cap ){
		mtx.unlock();
		return 0;
	}

	list[head] = entry;
	head =(head+1) % cap;

	mtx.unlock();
	return 1;
}

template<class T> int Queue_l_mpmc<T>::pop(T& dest){

	mtx.lock();
	
	if(tail == head){
		mtx.unlock();
		return 0;
	}

	dest = list[tail];
	tail = (tail + 1 ) % cap;

	mtx.unlock();
	return 1;
}

#endif /* L_QUEUE_MPMC_H_ */
