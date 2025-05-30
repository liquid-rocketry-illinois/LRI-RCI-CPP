#ifndef RINGBUFFER_INL
#define RINGBUFFER_INL

// Linters will complain this isnt needed but it really is for the class definition
#include "utils.h"

namespace LRI::RCI {
    // RingBuffer template implementation

    // New buffer constructor. Takes in a size for the buffer. Initializes all variables and allocates memory
    template<typename T, T ret>
    RingBuffer<T, ret>::RingBuffer(uint32_t _buffersize) :
        buffersize(_buffersize + 1), datastart(0), dataend(0), data(nullptr) {
        data = new T[buffersize];
        memset(data, ret, buffersize);
    }

    // Copy constructor. Performs a deep copy of memory segment with data.
    template<typename T, T ret>
    RingBuffer<T, ret>::RingBuffer(RingBuffer& other) :
        buffersize(other.buffersize), datastart(other.datastart), dataend(other.dataend), data(nullptr) {
        data = new T[buffersize];
        memcpy(data, other.data, buffersize);
    }

    // Destructor just needs to delete the data memory segment in heap
    template<typename T, T ret>
    RingBuffer<T, ret>::~RingBuffer() {
        delete[] data;
    }

    template<typename T, T ret>
    bool RingBuffer<T, ret>::isEmpty() const {
        return datastart == dataend;
    }

    template<typename T, T ret>
    bool RingBuffer<T, ret>::isFull() const {
        return datastart == dataend + 1;
    }

    template<typename T, T ret>
    uint32_t RingBuffer<T, ret>::size() const {
        return dataend >= datastart ? dataend - datastart : buffersize - (datastart - dataend);
    }

    // Capacity is not size
    template<typename T, T ret>
    uint32_t RingBuffer<T, ret>::capacity() const {
        return buffersize - 1;
    }

    // Pops a value from the buffer
    template<typename T, T ret>
    T RingBuffer<T, ret>::pop() {
        if(isEmpty()) return ret;
        T retval = data[datastart];
        datastart = (datastart + 1) % buffersize;
        return retval;
    }

    // Returns the value at the front of the buffer but does not remove it
    template<typename T, T ret>
    T RingBuffer<T, ret>::peek() const {
        if(isEmpty()) return ret;
        return data[datastart];
    }

    // Pushes a new value to the buffer. If the buffer is full, it overwrites the next value
    template<typename T, T ret>
    void RingBuffer<T, ret>::push(T value) {
        data[dataend] = value;
        dataend = (dataend + 1) % buffersize;

        // If the buffer is full move the start index one forward
        if(dataend == datastart) datastart = (datastart + 1) % buffersize;
    }

    template<typename T, T ret>
    void RingBuffer<T, ret>::clear() {
        datastart = 0;
        dataend = 0;
    }
} // namespace LRI::RCI

#endif
