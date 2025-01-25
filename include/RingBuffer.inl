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
    RingBuffer<T, ret>::RingBuffer(RingBuffer& other) : buffersize(other.buffersize), datastart(other.datastart),
                                                        dataend(other.dataend), data(nullptr) {
        data = new T[buffersize];
        memcpy(data, other.data, buffersize);
    }

    template<typename T, T ret>
    RingBuffer<T, ret>::~RingBuffer() {
        delete[] data;
    }

    template<typename T, T ret>
    uint32_t RingBuffer<T, ret>::size() const {
        return dataend - datastart;
    }

    template<typename T, T ret>
    uint32_t RingBuffer<T, ret>::capacity() const {
        return buffersize - 1;
    }

    template<typename T, T ret>
    T RingBuffer<T, ret>::pop() {
        if(size() == 0) return ret;
        T retval = data[datastart];
        datastart = (datastart + 1) % buffersize;
        return retval;
    }

    template<typename T, T ret>
    T RingBuffer<T, ret>::peek() const {
        if(size() == 0) return ret;
        return data[datastart];
    }

    template<typename T, T ret>
    void RingBuffer<T, ret>::push(T value) {
        data[dataend] = value;
        dataend = (dataend + 1) % buffersize;
        if(dataend == datastart) datastart = (datastart + 1) % buffersize;
    }
}

#endif
