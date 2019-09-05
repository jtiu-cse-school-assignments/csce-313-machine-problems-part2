#ifndef _BOUNDEDBUFFER_H_  
#define _BOUNDEDBUFFER_H_


#include <queue>
#include "semaphore.H"
#include <pthread.h>
#include <iostream>

class BoundedBuffer {
    
private:
    
    std::queue<std::string> bbQueue;
    Semaphore mutex;
    Semaphore empty;
    Semaphore full;
    
public:
    
    BoundedBuffer();
    BoundedBuffer(int _full) : mutex(1), empty(0), full(_full) {}
    void toBuffer(std::string request);
    std::string fromBuffer();
    void size();
    
};

#endif