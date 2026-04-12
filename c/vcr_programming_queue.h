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
    int buffer_length;
    int read_index;
    int write_index;
} VcrProgrammingQueue;

void safe_copy_program(VcrProgram *dest, VcrProgram *src);
/*
 * Function to initialize the queue. Can be used to reset/clear the queue
 */
void initialize_queue(VcrProgrammingQueue *queue);
/*
 * Function to check if the queue is empty
 */
bool is_empty(VcrProgrammingQueue *queue);
/*
 * Function to check if the queue is full
 */
bool is_full(VcrProgrammingQueue *queue);
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

void safe_copy_program(VcrProgram *dest, VcrProgram *src) { strncpy(dest->url, src->url, MAX_STRING_LENGTH); }

void initialize_queue(VcrProgrammingQueue *queue) {
    queue->buffer_length = 0;
    queue->read_index = 0;
    queue->write_index = 0;
}

bool is_empty(VcrProgrammingQueue *queue) { return queue->buffer_length == 0; }

bool is_full(VcrProgrammingQueue *queue) { return queue->buffer_length == BUFFER_SIZE; }

EnqueueCode enqueue(VcrProgrammingQueue *queue, VcrProgram *program) {
    if (is_full(queue)) {
        return FAIL_QUEUE_FULL;
    }

    if (strlen(program->url) > MAX_STRING_LENGTH) {
        return FAIL_URL_LENGTH;
    }

    safe_copy_program(&queue->buffer[queue->write_index], program);
    queue->write_index = (queue->write_index + 1) % BUFFER_SIZE;
    queue->buffer_length++;

    return SUCCESS;
}

bool dequeue(VcrProgrammingQueue *queue, VcrProgram *program) {
    if (is_empty(queue)) {
        return false;
    }

    safe_copy_program(program, &queue->buffer[queue->read_index]);
    queue->read_index = (queue->read_index + 1) % BUFFER_SIZE;
    queue->buffer_length--;

    return true;
};
#endif 
