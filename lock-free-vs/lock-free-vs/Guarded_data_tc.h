
#ifndef GUARDED_DATA_TC
#define GUARDED_DATA_TC

#include "stdafx.h"

using namespace std;

//template <typename T> 
//struct Guarded_data{
//	atomic <int> g;
//	void* data;
//};
//typedef int T;

template <class T>
class Guarded_data_tc{
private:
	T data;
	atomic< int> guard{ 0 };
public:

	Guarded_data_tc() {
		guard.store(0, memory_order_release);
	}
	int inline tryPublish(T);
	void inline publish(T);

	void inline get(T&);
	int inline tryGet(T&);

	void inline rescind();
};


template<typename T> int inline Guarded_data_tc<T>::tryPublish(T entry){

	if (!guard.load(memory_order_acquire)){
		data = entry;
		guard.store(1, memory_order_release);
		return 1;
	}
	//printf("try pub: %u", guard.load());
	return 0;
}

template<typename T> void inline Guarded_data_tc<T>::get(T& dest){
	dest = data;
}


template <class T> void inline Guarded_data_tc<T>::publish(T entry){
//void inline Guarded_data_tc::publish(T entry){
	data = entry;
	guard.store(1, memory_order_release);
}

template <class T>int inline Guarded_data_tc<T>::tryGet(T& dest){
//int inline Guarded_data_tc::tryGet(T& dest){

	if (guard.load(memory_order_acquire)){//
		dest = data;
		return 1;
	}
	return 0;
}
template <class T>void inline Guarded_data_tc<T>::rescind(){
//void inline Guarded_data_tc::rescind(){
	guard.store(0, memory_order_release);
}
#endif
