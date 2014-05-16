
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
	}
	void inline publish(T data);
	int inline tryGet(T& dest);
	void inline rescind();
};


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
