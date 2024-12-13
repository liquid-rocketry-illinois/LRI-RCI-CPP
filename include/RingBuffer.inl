#ifndef RINGBUFFER_INL
#define RINGBUFFER_INL

#include "utils.h"

namespace LRI::RCI {
    template<typename T, T ret>
    RingBuffer<T, ret>::RingBuffer(uint32_t _buffersize) : buffersize(_buffersize), datastart(0), dataend(0),
                                                           data(nullptr) {
        data = new T[buffersize];
        memset(data, 0, buffersize);
    }

    template<typename T, T ret>
    RingBuffer<T, ret>::~RingBuffer() {
        delete[] data;
    }

    template<typename T, T ret>
    uint32_t RingBuffer<T, ret>::length() const {
        return datastart - dataend;
    }

    template<typename T, T ret>
    uint32_t RingBuffer<T, ret>::size() const {
        return buffersize;
    }

    template<typename T, T ret>
    T RingBuffer<T, ret>::pop() {
        if(length() == 0) return ret;
        T retval = data[datastart % buffersize];
        datastart++;
        return retval;
    }

    template<typename T, T ret>
    T RingBuffer<T, ret>::peek() const {
        if(length() == 0) return ret;
        return data[datastart % buffersize];
    }

    template<typename T, T ret>
    void RingBuffer<T, ret>::push(T value) {
        data[dataend] = value;
        dataend++;
    }
}

#endif