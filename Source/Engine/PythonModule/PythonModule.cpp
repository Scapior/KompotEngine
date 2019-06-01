#include "PythonModule.hpp"

using namespace KompotEngine;


PythonModule::PythonModule(std::shared_ptr<World> &world)
    : TickingObject(2000ms)//33ms
{
    m_world = world;

    PyImport_AppendInittab("KompotEngine", &PythonModule::initPythonModule);
    Py_Initialize();
    if (PyErr_Occurred())
    {
        PyErr_Print();
        PyErr_Clear();
    }
    PyRun_SimpleString("import sys, os");
    PyRun_SimpleString("sys.path.append(os.getcwd())");
    PyRun_SimpleString("import KompotEngine");
}



PythonModule::~PythonModule()
{
    Py_Finalize();
}

void PythonModule::tick()
{
    m_world->lock();
    for (auto &object : m_world->getMeshObjects())
    {
        PythonObject unicodeFileName = PyUnicode_FromString(object->getScriptModuleName().c_str());
        PythonObject pythonScriptModule = PyImport_Import(unicodeFileName);
        if (pythonScriptModule)
        {
            static PyObject* argsEmpty = PyTuple_New(0);
            PythonObject kwargs = Py_BuildValue("{s:K,s:K}",
                                                 "objectId", object->getObjectId(),
                                                 "worldId", reinterpret_cast<uint64_t>(m_world.get()));
            PythonObject onTickPythonFuntion = PyObject_GetAttrString(pythonScriptModule, "onTick");
            PythonObject pValue = PyObject_Call(onTickPythonFuntion, argsEmpty, kwargs);
            if (PyErr_Occurred())
            {
                PyErr_Print();
                PyErr_Clear();
            }
        }
        else if (PyErr_Occurred())
        {
            Log::getInstance() << "PyImport_Import error" << std::endl;
            PyErr_Print();
            PyErr_Clear();
        }
    }
    m_world->unlock();
}

PyObject *PythonModule::initPythonModule()
{
    static PyMethodDef pythonMethods [] = {
        {"log", &PythonModule::m_python_log, METH_VARARGS, "Print  text to the log"},
        {"addObject", &PythonModule::m_python_addObject, METH_VARARGS, "Add new object of passed class to world"},
        {nullptr, nullptr, 0, nullptr}
    };

    static PyModuleDef pythonModule= {
        PyModuleDef_HEAD_INIT,
        "KompotEngine",          // m_name
        "KompotEngine funtions", // m_doc
        2,                       // m_size
        pythonMethods,           // m_methods
        nullptr,                 // m_reload
        nullptr,                 // m_traverse
        nullptr,                 // m_clear
        nullptr,                 // m_free
    };

    return PyModule_Create(&pythonModule);
}

PyObject * PythonModule::m_python_log(PyObject*, PyObject* args)
{
    const char *toPrint;
    if(!PyArg_ParseTuple(args, "s", &toPrint))
    {
        return nullptr;
    }
    Log::getInstance() << toPrint << std::endl;
    Py_RETURN_NONE;
}

PyObject * PythonModule::m_python_addObject(PyObject*, PyObject* args)
{
    uint64_t worldPointerContainer = 0;
    const char *className;
    if(!PyArg_ParseTuple(args, "Ks", &worldPointerContainer, &className))
    {
        return nullptr;
    }
    auto world = reinterpret_cast<World*>(worldPointerContainer);
    auto object = world->createObject(className);
    auto pos = static_cast<float>(object->getObjectId() + 1) * -0.1f;
    object->scale(glm::vec3(0.1f));
    object->moveTo(glm::vec3(pos));
    return Py_BuildValue("K", object->getObjectId());
}
