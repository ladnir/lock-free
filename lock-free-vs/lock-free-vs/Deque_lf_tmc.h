/* 
 * A lock free deque what has a spsc head and a tail that is mc.
 * The head can be access by ONE thread and the tail can be accessed
 * by many. 
 *                      head         tail
 *   single push/popper [#, #, #, ..., #] multi poppers
 */
#pragma once
#include "Guarded_data_tc.h"

template<typename T> class Deque_lf_tmc
{
	Guarded_data_tc<T>* list;
	unsigned int cap;
	unsigned int capMask;
	atomic<unsigned int> tail;
	atomic<unsigned int> head;

	int isPowerOfTwo(unsigned int);

public:
	Deque_lf_tmc(unsigned int);
	~Deque_lf_tmc();

	int pushHead(T);
	int popHead(T&);
	int popTail(T&);
};

