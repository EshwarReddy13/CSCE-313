#include "BoundedBuffer.h"

using namespace std;


BoundedBuffer::BoundedBuffer (int _cap) : cap(_cap) {
    // modify as needed
}

BoundedBuffer::~BoundedBuffer () {
    // modify as needed
}

void BoundedBuffer::push (char* msg, int size) {
    // 1. Convert the incoming byte sequence given by msg and size into a vector<char>
    vector<char> data(msg,msg+size);
    // 2. Wait until there is room in the queue (i.e., queue lengh is less than cap)
    unique_lock<mutex> lck(mut);
    slot_available.wait(lck,[this]{return (int)q.size()<cap;});
    // 3. Then push the vector at the end of the queue
    q.push(data);
    lck.unlock();
    // 4. Wake up threads that were waiting for push
    data_available.notify_one();
}

int BoundedBuffer::pop (char* msg, int size) {
    unique_lock<mutex> lck(mut);
    // 1. Wait until the queue has at least 1 item
    data_available.wait(lck,[this]{return q.size()>0;});
    // 2. Pop the front item of the queue. The popped item is a vector<char>
    vector<char> temp = q.front();
    int vecSize = temp.size();
    q.pop();
    // 3. Convert the popped vector<char> into a char*, copy that into msg; assert that the vector<char>'s length is <= size
    assert(vecSize<=size);
    memcpy(msg,temp.data(),vecSize);
    // 4. Wake up threads that were waiting for pop
    slot_available.notify_one();
    // 5. Return the vector's length to the caller so that they know how many bytes were popped
    lck.unlock();
    return vecSize;
}

size_t BoundedBuffer::size () {
    return q.size();
}