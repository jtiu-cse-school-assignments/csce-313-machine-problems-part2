#include "BoundedBuffer.H"

using namespace std;

void BoundedBuffer::toBuffer(string request) {
    full.P();
    mutex.P();
    bbQueue.push(request);
    mutex.V();
    empty.V();
}

string BoundedBuffer::fromBuffer() {
	empty.P();
    mutex.P();
    string temp = bbQueue.front();
    bbQueue.pop();
    mutex.V();
    full.V();
    
    return temp;
}

void BoundedBuffer::printBuffer() {
	queue<std::string> temp;
	
	temp = bbQueue;
	
	while(!temp.empty()) {
		cout << temp.front() << endl;
		temp.pop();
	}
}

bool BoundedBuffer::emptyy() {
	return bbQueue.empty();
}