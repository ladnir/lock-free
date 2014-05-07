///*
// * queue_spsc.h
// *
// *  Created on: May 4, 2014
// *      Author: Peter Rindal
// *
// *      Bounded single producer single consumer
// *      non-blocking lock free queue.
// *
// *      g++ --std=c++11 ...
// */
//#include "stdafx.h"
//
//#ifndef QUEUE_SPSC_H_
//#define QUEUE_SPSC_H_
//
//using namespace std;
//
//template <class T> class Queue_lf_spsc{
//public:
//	Queue_lf_spsc(int capacity){
//		cap = capacity;
//		list.reserve(cap);
//		head = 0;
//		tail = 0;
//	}
//	int push(T);
//	int pop(T&);
//
//private:
//	vector<T> list;
//	int cap;
//	atomic<int> head;
//	atomic<int> tail;
//};
//
//template<class T>int Queue_lf_spsc<T>::push(T entry){
//
//	int lcl_tail = tail.load(memory_order_acquire);
//	int lcl_head = head.load(memory_order_relaxed);
//
//	if (lcl_tail + cap - 1 == lcl_head )return 0;
//
//	list[lcl_head % cap] = entry;
//	head.store(lcl_head + 1, memory_order_release);
//
//	return 1;
//}
//
//template<class T> int Queue_lf_spsc<T>::pop(T& dest){
//	int lcl_tail = queue->tail.load(memory_order_relaxed);
//	int lcl_head = queue->head.load(memory_order_acquire);
//
//	if (lcl_tail == lcl_head) return 0;
//
//	dest = queue->list[lcl_tail % cap];
//	queue->tail.store(lcl_tail + 1, memory_order_release);
//
//	return 1;
//}
//
//
//#endif /* QUEUE_SPSC_H_ */
