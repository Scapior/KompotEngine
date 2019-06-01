#pragma once

#include "global.hpp"
#include "PythonObject.hpp"
#include "../../Misc/TickingObject.hpp"
#include "../World.hpp"
#include <Python.h>
#include <memory>
#include <string>
#include <functional>

namespace KompotEngine
{

//class

class PythonModule : public TickingObject
{
public:
    PythonModule(std::shared_ptr<World>&);
    ~PythonModule();
    void runScript(const std::string&);


private:
    std::shared_ptr<World> m_world;

    void tick() override;

    // python funtions

    static PyObject * initPythonModule();

    static PyObject * m_python_log(PyObject*,PyObject*);
    static PyObject * m_python_addObject(PyObject*, PyObject*);
};


} // KompotEngine namespace
