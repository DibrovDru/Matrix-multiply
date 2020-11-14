#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <malloc.h>
#include <stdlib.h>

void parseArray(long long size, PyObject* listOfLists, double*** matrix) {
    long long numberOfLines = PySequence_Fast_GET_SIZE(listOfLists);
    *matrix = malloc(size * sizeof(double*));
    long long i, j;

    for (i = 0; i < size; ++i) {
        (*matrix)[i] = malloc(size * sizeof(double));

        if (i < numberOfLines) {
            PyObject* line = PySequence_Fast_GET_ITEM(listOfLists, i);
            long long lenOfLine = PySequence_Fast_GET_SIZE(line);

            for (j = 0; j < size; ++j) {
                if (j < lenOfLine) {
                    PyObject* pyValue = PySequence_Fast_GET_ITEM(line, j);
                    PyObject* value = PyNumber_Float(pyValue);

                    (*matrix)[i][j] = PyFloat_AS_DOUBLE(value);
                } else {
                    (*matrix)[i][j] = 0.0;
                }
            }
        } else {
            for (j = 0; j < size; ++j) {
                (*matrix)[i][j] = 0.0;
            }
        }
    }
}

double** multiply(long long size, double*** firstMatrix, double*** secondMatrix) {
    double** result;
    result = malloc(size * sizeof(double*));
    long long i, j, l;

    for (i = 0; i < size; ++i) {
        result[i] = malloc(size * sizeof(double));

        for (j = 0; j < size; ++j) {
            result[i][j] = 0.0;

            for (l = 0; l < size; ++l) {
                result[i][j] += (*firstMatrix)[i][l] * (*secondMatrix)[l][j];
            }
        }
    }

    return result;
}

static PyObject* dot(PyObject* self, PyObject* args) {
    long long size = 0;
    double** firstMatrix;
    double** secondMatrix;
    PyObject* firstObj;
    PyObject* secondObj;

    if (!PyArg_ParseTuple(args, "LOO", &size, &firstObj, &secondObj)) {
        return NULL;
    }

    if (size <= 0) {
        PyErr_SetString(PyExc_TypeError, "invalid value");
        return NULL;
    }

    parseArray(size, firstObj, &firstMatrix);
    parseArray(size, secondObj, &secondMatrix);

    double** resultC = multiply(size, &firstMatrix, &secondMatrix);

    PyObject* resultPy = PyList_New(size);
    long long i, j;
    for (i = 0; i < size; ++i) {
        PyObject* currList = PyList_New(size);

        for (j = 0; j < size; ++j){
            PyObject* curr = Py_BuildValue("d", resultC[i][j]);
            PyList_SetItem(currList, j, curr);
        }
        PyList_SetItem(resultPy, i, currList);
    }

    for (i = 0; i < size; ++i) {
        free(firstMatrix[i]);
        free(secondMatrix[i]);
        free(resultC[i]);
    }

    free(firstMatrix);
    free(secondMatrix);
    free(resultC);

    return resultPy;
}

static PyMethodDef matrix_methods[] = {
        {"dot", (PyCFunction) dot, METH_VARARGS,"matrix multiply"},
        {NULL, NULL, 0, NULL}
};

static struct PyModuleDef matrix = {
        .m_base = PyModuleDef_HEAD_INIT,
        .m_name = "matrix",
        .m_methods = matrix_methods,
        .m_size = -1
};

PyMODINIT_FUNC PyInit_matrix(void) {
    return PyModule_Create(&matrix);
}
