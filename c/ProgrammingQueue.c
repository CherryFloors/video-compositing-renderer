#pragma once
#include <stdbool.h>
#include <string.h>
#include <stdatomic.h>

#define BUFFER_SIZE 100
#define MAX_STRING_LENGTH 512
#define STRING_BUFFER_WIDTH (MAX_STRING_LENGTH + 1)


typedef enum {
    SUCCESS,
    FAIL_QUEUE_FULL,
    FAIL_URL_LENGTH,
} EnqueueCode;


// Program Struct
typedef struct {
    char url[STRING_BUFFER_WIDTH];
} Program;


// Defining the Queue structure
typedef struct {
    Program buffer[BUFFER_SIZE];
    int buffer_length;
    int read_index;
    int write_index;
} ProgrammingQueue;


void safe_copy_program(Program* dest, Program* src) {
    strncpy(dest->url, src->url, MAX_STRING_LENGTH);
}


/*
 * Function to initialize the queue. Can be used to reset/clear the queue
 */
void initialize_queue(ProgrammingQueue* q)
{
    q->buffer_length = 0;
    q->read_index = 0;
    q->write_index = 0;
}


/*
 * Function to check if the queue is empty
 */
bool is_empty(ProgrammingQueue* q) { return q->buffer_length == 0; }


/*
 * Function to check if the queue is full
 */
bool is_full(ProgrammingQueue* q) { return q->buffer_length == BUFFER_SIZE; }


/*
 * Enqueues the given void* element at the back of this Queue.
 * Returns true on success and false on enq failure when element is NULL or queue is full.
 */
EnqueueCode enqueue(ProgrammingQueue* q, Program* program_p) {

    if (is_full(q)) {
        return FAIL_QUEUE_FULL;
    }

    if (strlen(program_p->url) > MAX_STRING_LENGTH) {
        return FAIL_URL_LENGTH;
    }

    safe_copy_program(&q->buffer[q->write_index], program_p);
    q->write_index = (q->write_index + 1) % BUFFER_SIZE;
    q->buffer_length++;

    return SUCCESS;
}


/*
 * Some Docs...
 */
bool dequeue(ProgrammingQueue* q, Program* program_p) {

    if (is_empty(q)) {
        return false;
    }

    safe_copy_program(program_p, &q->buffer[q->read_index]);
    q->read_index = (q->read_index + 1) % BUFFER_SIZE;
    q->buffer_length--;

    return true;

};
