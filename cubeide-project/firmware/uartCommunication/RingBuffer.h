/*
 * RingBuffer.h
 *
 *  Created on: Oct 8, 2025
 *      Author: tizianohumpert
 */

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/**
 * Template-basierter Ringbuffer für beliebige Datentypen.
 * Für UART: T = uint8_t
 */
template<typename T, uint16_t Size>
class RingBuffer {
public:
    RingBuffer() : head(0), tail(0), overflow(false) {}

    bool put(T data) {
        uint16_t next = (head + 1) % Size;
        if (next == tail) {
            overflow = true; // Buffer voll
            return false;
        }
        buffer[head] = data;
        head = next;
        return true;
    }

    bool get(T &data) {
        if (isEmpty()) return false;
        data = buffer[tail];
        tail = (tail + 1) % Size;
        return true;
    }

    uint16_t available() const {
        if (head >= tail)
            return head - tail;
        else
            return Size - tail + head;
    }

    bool isEmpty() const {
        return head == tail;
    }

    bool isFull() const {
        return ((head + 1) % Size) == tail;
    }

    void reset() {
        head = tail = 0;
        overflow = false;
    }

    bool hadOverflow() const {
        return overflow;
    }

private:
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile bool overflow;
    T buffer[Size];
};

#endif /* RINGBUFFER_H_ */
