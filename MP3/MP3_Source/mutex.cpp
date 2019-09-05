#include "mutex.H"
  
Mutex::Mutex() {
	pthread_mutex_init(&m, NULL);
}
  
void Mutex::Lock() {
	pthread_mutex_lock(&m);
}
void Mutex::Unlock() {
	pthread_mutex_unlock(&m);
}