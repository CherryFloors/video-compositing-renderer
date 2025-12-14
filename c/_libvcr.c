#include <python3.12/Python.h>
#include <python3.12/longobject.h>
#include <python3.12/unicodeobject.h>

#include "ProgrammingQueue.c"

ProgrammingQueue programming_queue;

static PyObject *_queue_count(PyObject *self, PyObject *args) {
    return PyLong_FromLong(programming_queue.buffer_length);
}

static PyObject *_clear_queue(PyObject *self, PyObject *args) {
    initialize_queue(&programming_queue);
    return PyLong_FromLong(programming_queue.buffer_length);
}

static PyObject *_enqueue_program(PyObject *self, PyObject *args) {
    Program program;
    char *program_url;

    /* Parse arguments */
    if (!PyArg_ParseTuple(args, "s", &program_url)) {
        return NULL;
    }

    strncpy(program.url, program_url, MAX_STRING_LENGTH);
    return PyLong_FromLong(enqueue(&programming_queue, &program));
}

static PyObject *_test_dequeue_program(PyObject *self, PyObject *args) {
    Program program;
    if (dequeue(&programming_queue, &program)) {
        return PyUnicode_DecodeASCII(program.url, strlen(program.url), "strict");
    }

    return PyUnicode_DecodeASCII("", strlen(""), "strict");
}

static PyMethodDef libvcrmethods[] = {
    {"_queue_count", _queue_count, METH_NOARGS, "Check the current Program Queue Count."},
    {"_clear_queue", _clear_queue, METH_NOARGS, "Clear the Program Queue."},
    {"_enqueue_program", _enqueue_program, METH_VARARGS, "Add a program to the queue if there is room."},
    {"_test_dequeue_program", _test_dequeue_program, METH_VARARGS, "Add a program to the queue if there is room."},
    {NULL, NULL, 0, NULL},
};

static struct PyModuleDef libvcrmodule = {
    PyModuleDef_HEAD_INIT, "_libvcr", "Python interface for the vcr C API", -1, libvcrmethods,
};

PyMODINIT_FUNC PyInit__libvcr(void) { return PyModule_Create(&libvcrmodule); }
