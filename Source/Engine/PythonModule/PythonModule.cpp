#include "PythonModule.hpp"

using namespace KompotEngine;


PythonModule::PythonModule(std::shared_ptr<World> &world)
    : TickingObject(2000ms),
      m_mainScript(nullptr)//33ms
{
    m_world = world;

    PyImport_AppendInittab("KompotEngine", &PythonModule::initPythonModule);
    Py_Initialize();
    PyRun_SimpleString("import sys, os");
    PyRun_SimpleString("sys.path.append(os.getcwd())");
    PyRun_SimpleString("import KompotEngine");
    if (PyErr_Occurred())
    {
        PyErr_Print();
        PyErr_Clear();
    }

    m_mainScript = loadModule("main");
    m_mainScriptOnTickFuntion = PyObject_GetAttrString(m_mainScript, "onTick");
//    PythonObject mainScriptOnInit = PyObject_GetAttrString(m_mainScript, "OnInit");
}



PythonModule::~PythonModule()
{
    Py_Finalize();
}

PythonObject PythonModule::loadModule(const std::string &moduleName)
{
    PythonObject pythonScriptModule = nullptr;
    PythonObject unicodeFileName = PyUnicode_FromString(moduleName.c_str());
    pythonScriptModule = PyImport_Import(unicodeFileName);
    if (!pythonScriptModule)
    {
        Log::getInstance() << "PyImport_Import error for script module "<< moduleName << std::endl;
        PyErr_Print();
        PyErr_Clear();
    }
    return pythonScriptModule;
}

void PythonModule::tick()
{
//    for (const auto& objectId : m_objectsToDelete)
//    {
//        m_modulesCache.erase(objectId);
//    }
//    m_objectsToDelete.clear();

    m_world->lock();

    static PyObject* argsEmpty = PyTuple_New(0);
    PythonObject kwargs = Py_BuildValue("{s:K}",
                                         "worldId", reinterpret_cast<uint64_t>(m_world.get()));

    PyObject_Call(m_mainScriptOnTickFuntion, argsEmpty, kwargs);

    for (auto&& [objectId, object] : m_world->getMeshObjects())
    {
        PythonObject pythonScriptModule = nullptr;

        if (m_modulesCache.find(objectId) == m_modulesCache.end())
        {
            pythonScriptModule = loadModule(object->getScriptModuleName());
            if(!pythonScriptModule)
            {
                m_modulesCache[objectId] = pythonScriptModule;
            }
        }
        else
        {
            pythonScriptModule = m_modulesCache[objectId];
        }
        if (pythonScriptModule)
        {            
            PythonObject kwargs = Py_BuildValue("{s:K,s:K}",
                                                 "objectId", objectId,
                                                 "worldId", reinterpret_cast<uint64_t>(m_world.get()));
            PythonObject onTickPythonFuntion = PyObject_GetAttrString(pythonScriptModule, "onTick");
            if (!onTickPythonFuntion || PyErr_Occurred())
            {
                PyErr_Print();
                PyErr_Clear();
            }
            else
            {
                PythonObject pValue = PyObject_Call(onTickPythonFuntion, argsEmpty, kwargs);
            }
        }        
    }
    m_world->unlock();
}

PyObject *PythonModule::initPythonModule()
{
    static std::vector<PyMethodDef> pythonMethods = {
        {"log", &PythonModule::m_python_log, METH_VARARGS, "Print  text to the log"},
        {"addObject", &PythonModule::m_python_addObject, METH_VARARGS, "Add new object of passed class to world"},
        {"deleteObject", &PythonModule::m_python_deleteObject, METH_VARARGS, "Remove object by ID"},
        {"moveObjectTo", &PythonModule::m_python_moveObjectTo, METH_VARARGS, "Move object"},
        {"rotateObject", &PythonModule::m_python_rotateObject, METH_VARARGS, "Rotate object"},
        {"scaleObject", &PythonModule::m_python_scaleObject, METH_VARARGS, "Scale object"},
        {nullptr, nullptr, 0, nullptr}
    };

    static PyModuleDef pythonModule= {
        PyModuleDef_HEAD_INIT,
        "KompotEngine",          // m_name
        "KompotEngine funtions", // m_doc
        static_cast<long>(pythonMethods.size()) - 1l, // m_size
        pythonMethods.data(),    // m_methods
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
    return Py_BuildValue("K", object->getObjectId());
}

PyObject * PythonModule::m_python_deleteObject(PyObject*, PyObject* args)
{
    uint64_t worldPointerContainer = 0;
    uint64_t objectId = 0;
    if(!PyArg_ParseTuple(args, "KK", &worldPointerContainer, &objectId))
    {
        return nullptr;
    }
    auto world = reinterpret_cast<World*>(worldPointerContainer);
    world->deleteObject(objectId);
    Py_RETURN_NONE;
}

PyObject * PythonModule::m_python_moveObjectTo(PyObject*, PyObject* args)
{
    uint64_t worldPointerContainer = 0;
    uint64_t objectId = 0;
    float x,y,z;
    if(!PyArg_ParseTuple(args, "KKfff", &worldPointerContainer, &objectId, &x, &y, &z))
    {
        return nullptr;
    }
    auto world = reinterpret_cast<World*>(worldPointerContainer);
    auto object = world->getObjectById(objectId);
    object->moveTo(glm::vec3(x,y,z));
    Py_RETURN_NONE;
}

PyObject * PythonModule::m_python_rotateObject(PyObject *, PyObject * args)
{
    uint64_t worldPointerContainer = 0;
    uint64_t objectId = 0;
    float x,y,z;
    if(!PyArg_ParseTuple(args, "KKfff", &worldPointerContainer, &objectId, &x, &y, &z))
    {
        return nullptr;
    }
    auto world = reinterpret_cast<World*>(worldPointerContainer);
    auto object = world->getObjectById(objectId);
    object->rotate(glm::vec3(x,y,z));
    Py_RETURN_NONE;
}

PyObject * PythonModule::m_python_scaleObject(PyObject *, PyObject * args)
{
    uint64_t worldPointerContainer = 0;
    uint64_t objectId = 0;
    float x,y,z;
    if(!PyArg_ParseTuple(args, "KKfff", &worldPointerContainer, &objectId, &x, &y, &z))
    {
        return nullptr;
    }
    auto world = reinterpret_cast<World*>(worldPointerContainer);
    auto object = world->getObjectById(objectId);
    if (object)
    object->scale(glm::vec3(x,y,z));
    Py_RETURN_NONE;
}
