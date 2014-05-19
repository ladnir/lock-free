#include "stdafx.h"
#include "Queue_lf_spmc.h"


template<typename T> Queue_lf_spmc<T>::Queue_lf_spmc(unsigned int capacity)
{
	assert(capacity != 0 && "capacity must be less Max unsigned int");
	list = (Guarded_data_tc<T>*)malloc(sizeof(Guarded_data_tc<T>)*capacity);
	cap = capacity;
	unsigned int i;
	for (i = 0; i < cap; i++){
		list[i].rescind();
	}
	tail.store(0, memory_order_release);
	head.store(0, memory_order_release);
}


template<typename T> Queue_lf_spmc<T>::~Queue_lf_spmc()
{
	free(list);
}

template<typename T> int Queue_lf_spmc<T>::push(T entry)
{
	unsigned int lcl_tail, lcl_head;

	lcl_tail = tail.load(memory_order_relaxed);
	lcl_head = head.load(memory_order_relaxed);
	//printf("push tail %u, head %u\n", lcl_tail, lcl_head);
	if (lcl_tail != lcl_head - cap) { // not full
	
		if (list[lcl_head % cap].tryPublish(entry)){
			head.store(lcl_head+1, memory_order_release);
			return 1;
		}
		//printf("try pub failed\n");
	}

	//printf("full\n");
	return 0;
}
template<typename T> int Queue_lf_spmc<T>::pop(T& dest)
{
	unsigned int lcl_tail, lcl_head;

	do{
		lcl_tail = this->tail.load(memory_order_relaxed);
		lcl_head = this->head.load(memory_order_relaxed);
		if (lcl_tail == lcl_head){ // head - cap == tail
			//printf("empty\n");
			return 0;
		}
		// try and claim the tail for our selves
	} while (!atomic_compare_exchange_weak_explicit(&tail,
													&lcl_tail,
													lcl_tail + 1,
													memory_order_release,
													memory_order_relaxed));

	// the tail is ours.
	Guarded_data_tc<T>* gd = &list[lcl_tail % cap];
	gd->get(dest);
	//printf("poping at %d, %d ,%d \n", (int)lcl_tail,(int)lcl_head, dest);
	gd->rescind();

	return 1;
}