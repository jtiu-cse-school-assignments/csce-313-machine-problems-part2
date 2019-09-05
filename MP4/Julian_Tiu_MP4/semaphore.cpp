#include "semaphore.H"

Semaphore::Semaphore(int _val) {
	value = _val;
}

Semaphore::~Semaphore() {
	pthread_mutex_destroy(&m);
	pthread_cond_destroy(&c);
}

int Semaphore::P() {
	
	pthread_mutex_lock(&m);
	value--;
	
	 /* if the value is 0, then there are no threads blocked in the queue */
	if(value < 0) {
		pthread_cond_wait(&c, &m);
	}
    
    pthread_mutex_unlock(&m);
	
	return 0;
}

int Semaphore::V() {
	
	pthread_mutex_lock(&m);
	value++;
	
	/*  */
	if(value <= 0) {
		pthread_cond_signal(&c);
	}
	
	pthread_mutex_unlock(&m);
	
	return 0;
}