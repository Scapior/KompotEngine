#include "PythonObject.hpp"

using namespace KompotEngine;

PythonObject::PythonObject()
    : m_pyObject(nullptr)
{}

PythonObject::PythonObject(PyObject *pyObject)
    : m_pyObject(pyObject)
{}


PythonObject::~PythonObject()
{
    if (m_pyObject)
    {
        Py_DECREF(m_pyObject);
    }
    m_pyObject = nullptr;
}

KompotEngine::PythonObject::operator PyObject *()
{
    return m_pyObject;
}
