#pragma once
#include <stdatomic.h>
#include <stdbool.h>
#include <string.h>

#define BUFFER_SIZE 100
#define MAX_STRING_LENGTH 512
#define STRING_BUFFER_WIDTH (MAX_STRING_LENGTH + 1)

typedef enum {
    SUCCESS,
    FAIL_QUEUE_FULL,
    FAIL_URL_LENGTH,
} EnqueueCode;

// Program Struct
typedef struct VcrProgram {
    char url[STRING_BUFFER_WIDTH];
} VcrProgram;

// Defining the Queue structure
typedef struct VcrProgrammingQueue {
    VcrProgram buffer[BUFFER_SIZE];
    _Atomic int read_index;
    _Atomic int write_index;
} VcrProgrammingQueue;

void safe_copy_program(VcrProgram *dest, VcrProgram *src);
/*
 * Function to initialize the queue. Can be used to reset/clear the queue
 */
void initialize_queue(VcrProgrammingQueue *queue);
/*
 * Function to get the queue count. Not atomic.
 */
int queue_count(VcrProgrammingQueue *queue);
/*
 * Enqueues the given void* element at the back of this Queue.
 * Returns true on success and false on enq failure when element is NULL or
 * queue is full.
 */
EnqueueCode enqueue(VcrProgrammingQueue *queue, VcrProgram *program_p);
/*
 * Some Docs...
 */
bool dequeue(VcrProgrammingQueue *queue, VcrProgram *program_p);


#ifdef VCR_PROGRAMMING_QUEUE_IMPLEMENTATION

void safe_copy_program(VcrProgram *dest, VcrProgram *src) {
    strncpy(dest->url, src->url, MAX_STRING_LENGTH); 
    dest->url[MAX_STRING_LENGTH] = '\0';
}

void initialize_queue(VcrProgrammingQueue *queue) {
    queue->read_index = 0;
    queue->write_index = 0;
}

int queue_count(VcrProgrammingQueue *queue) {

    int write_head = atomic_load_explicit(&queue->write_index, memory_order_acquire);
    int read_head = atomic_load_explicit(&queue->read_index, memory_order_acquire);

    if (write_head >= read_head) {
        return write_head - read_head;
    }
    
    return BUFFER_SIZE - read_head - write_head;
}

EnqueueCode enqueue(VcrProgrammingQueue *queue, VcrProgram *program) {

    if (strlen(program->url) > MAX_STRING_LENGTH) {
        return FAIL_URL_LENGTH;
    }

    int write_head = atomic_load_explicit(&queue->write_index, memory_order_relaxed);
    int next_write_head = (write_head + 1) % BUFFER_SIZE;

    if (next_write_head == atomic_load_explicit(&queue->read_index, memory_order_acquire)) {
        return FAIL_QUEUE_FULL;
    }

    safe_copy_program(&queue->buffer[write_head], program);
    atomic_store_explicit(&queue->write_index, next_write_head, memory_order_release);

    return SUCCESS;
}

bool dequeue(VcrProgrammingQueue *queue, VcrProgram *program) {

    int read_head = atomic_load_explicit(&queue->read_index, memory_order_relaxed);
    if (read_head == atomic_load_explicit(&queue->write_index, memory_order_acquire)) {
        return false;
    }

    safe_copy_program(program, &queue->buffer[read_head]);
    atomic_store_explicit(&queue->read_index, (read_head + 1) % BUFFER_SIZE, memory_order_release);

    return true;
};
#endif 
