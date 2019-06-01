#pragma once

#include "global.hpp"
#include <Python.h>


namespace KompotEngine
{

class PythonObject
{
public:
    PythonObject();
    PythonObject(PyObject*);
    ~PythonObject();

    operator PyObject* ();

private:
    PyObject *m_pyObject;
};

} // namespace KompotEngine
