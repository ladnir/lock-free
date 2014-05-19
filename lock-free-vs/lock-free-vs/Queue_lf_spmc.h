#pragma once
#include "stdafx.h"
#include "Guarded_data_tc.h"


template<typename T> class Queue_lf_spmc
{
	Guarded_data_tc<T>* list;
	unsigned int cap;
	atomic<unsigned int> tail;
	atomic<unsigned int> head;

public:
	Queue_lf_spmc(unsigned int capacity);
	~Queue_lf_spmc();

	int push(T);
	int pop(T&);
};


