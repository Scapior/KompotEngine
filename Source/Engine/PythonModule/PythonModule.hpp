#pragma once

#include "global.hpp"
#include "PythonObject.hpp"
#include "../../Misc/TickingObject.hpp"
#include "../World.hpp"
#include <Python.h>
#include <memory>
#include <string>
#include <functional>
#include <array>
#include <map>

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

    PythonObject loadModule(const std::string&);

    std::shared_ptr<World> m_world;
    PythonObject m_mainScript;
    PythonObject m_mainScriptOnTickFuntion;

    void tick() override;

    // python funtions

    static PyObject * initPythonModule();

    static PyObject * m_python_log(PyObject*,PyObject*);
    static PyObject * m_python_addObject(PyObject*, PyObject*);
    static PyObject * m_python_deleteObject(PyObject*, PyObject*);
    static PyObject * m_python_moveObjectTo(PyObject*, PyObject*);
    static PyObject * m_python_rotateObject(PyObject*, PyObject*);
    static PyObject * m_python_scaleObject(PyObject*, PyObject*);

    std::map<uint64_t, PythonObject> m_modulesCache;
    //std::vector<uint64_t> m_objectsToDelete;
};


} // KompotEngine namespace
