/*
 * Ring Buffer Library for Arduino
 *
 * Copyright Jean-Luc Béchennec 2018
 *
 * This software is distributed under the GNU Public Licence v2 (GPLv2)
 *
 * Please read the LICENCE file
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * Note about interrupt safe implementation
 *
 * To be safe from interrupts, a sequence of C instructions must be framed
 * by a pair of interrupt disable and enable instructions and ensure that the
 * compiler will not move writing of variables to memory outside the protected
 * area. This is called a critical section. Usually the manipulated variables
 * receive the volatile qualifier so that any changes are immediately written
 * to memory. Here the approach is different. First of all you have to know
 * that volatile is useless if the variables are updated in a function and
 * that this function is called within the critical section. Indeed, the
 * semantics of the C language require that the variables in memory be updated
 * before returning from the function. But beware of function inlining because
 * the compiler may decide to delete a function call in favor of simply
 * inserting its code in the caller. To force the compiler to use a real
 * function call, __attribute__((noinline)) have been added to the push and
 * pop functions. In this way the lockedPush and lockedPop functions ensure
 * that in the critical section a push and pop function call respectively will
 * be used by the compiler. This ensures that, because of the function call,
 * the variables are written to memory in the critical section and also
 * ensures that, despite the reorganization of the instructions due to
 * optimizations, the critical section will be well opened and closed at the
 * right place because function calls, due to potential side effects, are not
 * subject to such reorganizations.
 */

#ifndef __LRIRINGBUF_H__
#define __LRIRINGBUF_H__

#include <Arduino.h>

/*
 * Set the integer size used to store the size of the buffer according of
 * the size given in the template instanciation. Thanks to Niklas Gürtler
 * to share his knowledge of C++ template meta programming.
 * https://niklas-guertler.de/
 *
 * If Index argument is true, the ring buffer has a size and an index
 * stored in an uint8_t (Type below) because its size is within [1,255].
 * Intermediate computation may need an uint16_t (BiggerType below).
 * If Index argument is false, the ring buffer has a size and an index
 * stored in an uint16_t (Type below) because its size is within [256,65535].
 * Intermediate computation may need an uint32_t (BiggerType below).
 */
namespace LRI {
namespace RingBufHelper {
template<bool fits_in_uint8_t> struct Index {
  using Type = uint16_t;       /* index of the buffer */
  using BiggerType = uint32_t; /* for intermediate calculation */
};
template<> struct Index<false> {
  using Type = uint8_t;        /* index of the buffer */
  using BiggerType = uint16_t; /* for intermediate calculation */
};
} // namespace RingBufHelper

template<typename ET, size_t S, typename IT = typename RingBufHelper::Index<(S > 255)>::Type,
         typename BT = typename RingBufHelper::Index<(S > 255)>::BiggerType>
class RingBuf {
  /*
   * check the size is greater than 0, otherwise emit a compile time error
   */
  static_assert(S > 0, "RingBuf with size 0 are forbidden");

  /*
   * check the size is lower or equal to the maximum uint16_t value,
   * otherwise emit a compile time error
   */
  static_assert(S <= UINT16_MAX, "RingBuf with size greater than 65535 are forbidden");

 private:
  ET mBuffer[S];
  IT mReadIndex;
  IT mSize;

  IT writeIndex();
  void incReadIndex();

 public:
  /* Constructor. Init mReadIndex to 0 and mSize to 0 */
  RingBuf();
  /* Constructor with conditional helper. Does not init mReadIndex and mSize
   * if helper returns true */
  RingBuf(bool (*initHelper)(void));
  /* Push a data in the buffer and overwrite the older data if any */
  bool pushOverwrite(const ET inElement) __attribute__((noinline));
  /* Push a data in the buffer and overwrite the older data if any, copy it from
   * its pointer */
  bool pushOverwrite(const ET* const inElement) __attribute__((noinline));
  /* Push a data at the end of the buffer */
  bool push(const ET inElement) __attribute__((noinline));
  /* Push a data at the end of the buffer. Copy it from its pointer */
  bool push(const ET* const inElement) __attribute__((noinline));
  /* Push a data at the end of the buffer with interrupts disabled */
  bool lockedPush(const ET inElement);
  /* Push a data at the end of the buffer with interrupts disabled. Copy it from
   * its pointer */
  bool lockedPush(const ET* const inElement);
  /* Push a data in the buffer and overwrite the older data if any with
   * interrupts disabled */
  bool lockedPushOverwrite(const ET inElement);
  /* Push a data in the buffer and overwrite the older data if any with
   * interrupts disabled. Copy it from its pointer */
  bool lockedPushOverwrite(const ET* const inElement);
  /* Pop the data at the beginning of the buffer */
  bool pop(ET& outElement) __attribute__((noinline));
  /* Pop the data at the beginning of the buffer with interrupt disabled */
  bool lockedPop(ET& outElement);
  /* Return true if the buffer is full */
  bool isFull() {
    return mSize == S;
  }
  /* Return true if the buffer is empty */
  bool isEmpty() {
    return mSize == 0;
  }
  /* Reset the buffer  to an empty state */
  void clear() {
    mSize = 0;
  }
  /* return the size of the buffer */
  IT size() {
    return mSize;
  }
  /* return the maximum size of the buffer */
  IT maxSize() {
    return S;
  }
  /* access the buffer using array syntax, not interrupt safe */
  ET& operator[](IT inIndex);

  bool peek(ET& outElement, const size_t distance = 0) __attribute__((noinline));
  bool lockedPeek(ET& outElement, const size_t distance = 0);
};

template<typename ET, size_t S, typename IT, typename BT> IT RingBuf<ET, S, IT, BT>::writeIndex() {
  BT wi = (BT) mReadIndex + (BT) mSize;
  if(wi >= (BT) S) wi -= (BT) S;
  return (IT) wi;
}

template<typename ET, size_t S, typename IT, typename BT> void RingBuf<ET, S, IT, BT>::incReadIndex() {
  mReadIndex++;
  if(mReadIndex == S) mReadIndex = 0;
}

template<typename ET, size_t S, typename IT, typename BT> RingBuf<ET, S, IT, BT>::RingBuf() : mReadIndex(0), mSize(0) {}

template<typename ET, size_t S, typename IT, typename BT> RingBuf<ET, S, IT, BT>::RingBuf(bool (*initHelper)(void)) {
  if(!initHelper()) {
    mReadIndex = 0;
    mSize = 0;
  }
}

template<typename ET, size_t S, typename IT, typename BT> bool RingBuf<ET, S, IT, BT>::push(const ET inElement) {
  if(isFull()) return false;
  mBuffer[writeIndex()] = inElement;
  mSize++;
  return true;
}

template<typename ET, size_t S, typename IT, typename BT> bool RingBuf<ET, S, IT, BT>::push(const ET* const inElement) {
  if(isFull()) return false;
  mBuffer[writeIndex()] = *inElement;
  mSize++;
  return true;
}

template<typename ET, size_t S, typename IT, typename BT>
bool RingBuf<ET, S, IT, BT>::pushOverwrite(const ET inElement) {
  mBuffer[writeIndex()] = inElement;
  if(isFull()) {
    incReadIndex();
    return false;
  }
  else {
    mSize++;
    return true;
  }
}

template<typename ET, size_t S, typename IT, typename BT>
bool RingBuf<ET, S, IT, BT>::pushOverwrite(const ET* const inElement) {
  mBuffer[writeIndex()] = *inElement;
  if(isFull()) {
    incReadIndex();
    return false;
  }
  else {
    mSize++;
    return true;
  }
}

template<typename ET, size_t S, typename IT, typename BT> bool RingBuf<ET, S, IT, BT>::lockedPush(const ET inElement) {
  noInterrupts();
  bool result = push(inElement);
  interrupts();
  return result;
}

template<typename ET, size_t S, typename IT, typename BT>
bool RingBuf<ET, S, IT, BT>::lockedPush(const ET* const inElement) {
  noInterrupts();
  bool result = push(inElement);
  interrupts();
  return result;
}

template<typename ET, size_t S, typename IT, typename BT>
bool RingBuf<ET, S, IT, BT>::lockedPushOverwrite(const ET inElement) {
  noInterrupts();
  bool result = pushOverwrite(inElement);
  interrupts();
  return result;
}

template<typename ET, size_t S, typename IT, typename BT>
bool RingBuf<ET, S, IT, BT>::lockedPushOverwrite(const ET* const inElement) {
  noInterrupts();
  bool result = pushOverwrite(inElement);
  interrupts();
  return result;
}

template<typename ET, size_t S, typename IT, typename BT> bool RingBuf<ET, S, IT, BT>::pop(ET& outElement) {
  if(isEmpty()) return false;
  outElement = mBuffer[mReadIndex];
  incReadIndex();
  mSize--;
  return true;
}

template<typename ET, size_t S, typename IT, typename BT> bool RingBuf<ET, S, IT, BT>::lockedPop(ET& outElement) {
  noInterrupts();
  bool result = pop(outElement);
  interrupts();
  return result;
}

template<typename ET, size_t S, typename IT, typename BT>
bool RingBuf<ET, S, IT, BT>::peek(ET& outElement, const size_t distance) {
  if(isEmpty() || size() < distance) return false;
  // Take care of the wrap around
  size_t temp_read_index = mReadIndex + distance;
  if(temp_read_index >= S) {
    temp_read_index -= S;
  }

  outElement = mBuffer[temp_read_index];
  return true;
}

template<typename ET, size_t S, typename IT, typename BT>
bool RingBuf<ET, S, IT, BT>::lockedPeek(ET& outElement, const size_t distance) {
  noInterrupts();
  bool result = peek(outElement, distance);
  interrupts();
  return result;
}

template<typename ET, size_t S, typename IT, typename BT> ET& RingBuf<ET, S, IT, BT>::operator[](IT inIndex) {
  if(inIndex >= mSize) return mBuffer[0];
  BT index = (BT) mReadIndex + (BT) inIndex;
  if(index >= (BT) S) index -= (BT) S;
  return mBuffer[(IT) index];
}
} // namespace LRI

#endif /* __RINGBUF_H__ */
