#include "stdafx.h"
#include "Deque_lf_tmc.h"


template<typename T> Deque_lf_tmc<T>::Deque_lf_tmc(unsigned int capacity)
{
	assert(isPowerOfTwo(capacity) && "capacity must be a power  oof two.");
	
	list = (Guarded_data_tc<T>*)malloc(sizeof(Guarded_data_tc<T>)*capacity);
	cap = capacity;
	capMask = cap - 1;
	for (unsigned int i = 0; i < cap; i++)
		list[i].guard.store(0);

	tail.store(0, memory_order_release);
	head.store(0, memory_order_release);
}


template<typename T> Deque_lf_tmc<T>::~Deque_lf_tmc()
{
	free(list);
}

template <typename T> int Deque_lf_tmc<T>::isPowerOfTwo(unsigned int x)
{
	return ((x != 0) && !(x & (x - 1)));
}

template<typename T> int Deque_lf_tmc<T>::pushHead(T entry){
	
	unsigned int lcl_tail, lcl_head, lcl_guard;
	Guarded_data_tc<T>* gd;

	lcl_tail = tail.load(memory_order_acquire);
	lcl_head = head.load(memory_order_relaxed);
	//printf("push tail %u, head %u\n", lcl_tail, lcl_head);
	if (lcl_tail != lcl_head - cap) { // not full

		gd = &list[lcl_head & capMask];
		lcl_guard = gd->guard.load(memory_order_relaxed);
		if ((lcl_guard & 1) == 0){
			gd->data = entry;
			gd->guard.store(lcl_guard + 1, memory_order_release);

			head.store(lcl_head + 1, memory_order_relaxed);
			//printf("success\n");
			return 1;
		}
		printf("try pub failed %d\n",(int)lcl_guard);
		return 0;
	}
	printf("full\n");
	return 0;
}

template<typename T> int Deque_lf_tmc<T>::popHead(T& dest){

	unsigned int lcl_tail, lcl_head, lcl_guard;
	Guarded_data_tc<T>* gd;

	lcl_tail = tail.load(memory_order_relaxed);
	lcl_head = head.load(memory_order_relaxed);

	if (lcl_head == lcl_tail){ // empty?
		printf("head pop empty, h%d t%d\n",(int) lcl_head, (int) lcl_tail);
		return 0;
	}
	gd = &list[(lcl_head - 1) & capMask];
	lcl_guard = gd->guard.load(memory_order_relaxed);

	if ((lcl_guard & 1) == 0){ // is it being popped from the tail end?
		printf("head pop guard error\n");
		return 0;
	}
	if (atomic_compare_exchange_weak_explicit(&gd->guard,
											  &lcl_guard,
											  lcl_guard - 1,
											  memory_order_acquire,
											  memory_order_relaxed)){
		dest = gd->data;
		head.store(lcl_head - 1, memory_order_relaxed);
		return 1;
	}

	printf("head pop cas error\n");
	return 0;
}

template<typename T> int Deque_lf_tmc<T>::popTail(T& dest){
	unsigned int lcl_tail, lcl_head, lcl_guard;
	Guarded_data_tc<T>* gd;

	do{
		lcl_tail = tail.load(memory_order_relaxed);
		lcl_head = head.load(memory_order_relaxed);
		if (lcl_tail == lcl_head) // empty
			return 0;

		gd = &list[lcl_tail & capMask];
		lcl_guard = gd->guard.load(memory_order_relaxed);

		if ((lcl_guard & 1) == 0) // some one else is popping it
			continue;
		
		// try and claim the tail for our selves
	} while (!atomic_compare_exchange_weak_explicit(&gd->guard,
													&lcl_guard,
													lcl_guard + 1,
													memory_order_acquire,
													memory_order_relaxed));
	// the tail is ours.
	dest = gd->data;
	tail.store(lcl_tail + 1,memory_order_release);

	return 1;
}