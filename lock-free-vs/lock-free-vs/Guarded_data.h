
#include "stdafx.h"

using namespace std;

//template <typename T> 
struct Guarded_data{
	atomic <int> g;
	void* data;
};

//template <class T> class Guarded_data{
//public:
//	T data;
//	atomic< int> guard{ 0 };
//
//	//Guarded_data(const atomic<int>& zero) :
//	//	guard(zero) {}
//	//void inline publish(T data);
//	//int inline tryGet(T& dest);
//	//void inline rescind();
//};


//template <class T> void inline Guarded_data<T>::publish(T entry){
//	data = entry;
//	//guard.store(1, memory_order_release);
//}
//template <class T> int inline Guarded_data<T>::tryGet(T& dest){
//
//	if (1){//guard.load(memory_order_acquire)
//		dest = data;
//		return 1;
//	}
//	return 0;
//}
//template <class T> void inline Guarded_data<T>::rescind(){
//	//guard.store(0, memory_order_release);
//}
