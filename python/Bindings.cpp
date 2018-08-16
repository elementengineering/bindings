#include <cstdio>
#include <cstdlib>
#include <cassert>
#ifndef _NO_THREADS
#include <thread>
#endif //_NO_THREADS
#include <vector>
#include <stdint.h> // NOTE: Must be stdint.h not cstdint

// Fixes error with statically linked OpenSSL library
#ifdef MSVC
FILE _iob[] = {*stdin,*stdout,*stderr};
extern "C" FILE * __cdecl __iob_func(void) {return _iob;}
#endif

#include <Python.h>
// Include main uWebsockets
#include "uWS.h"





using namespace std;

#ifdef DEBUG_PYTHON_BINDINGS
    #define debug(...) printf(__VA_ARGS__)
#else
    #define debug(...)
#endif

/** Common Exception class **/
#ifndef MSVC
static PyObject * uWebSockets_error __attribute__ ((unused));
#else
static PyObject * uWebSockets_error;
#endif

//TODO: Remove, testing bindings work
static PyObject * uWebSockets_hello(PyObject * self, PyObject * args) {
    debug("Hello, world\n");
    Py_RETURN_NONE;
}

/**
 * Websocket extension
 */
template <bool isServer>
class WebSocket
{
    private: /** Don't define variables here it breaks Python, define at the bottom **/
        WebSocket() {
            // CPython cannot bind to constructors
            throw new runtime_error("Constructor cannot be called");
        }
        virtual ~WebSocket() {
            // CPython cannot bind to destructors
            throw new runtime_error("Destructor cannot be called");
        }
        
    public:
        /**
         * Wrapper for python __init__
         */
        int __init__(char * uri) {
            debug("Called __init__\n");
            this->hub = new uWS::Hub();
            this->ws = NULL;
            this->td = NULL;
            this->_state = INVALID;
            this->close_immediately = false;
            hub->uWS::Group<isServer>::onConnection([this](uWS::WebSocket<isServer> *ws, uWS::HttpRequest req) {
                debug("Connected!\n");
                if (this->close_immediately) {
                    ws->close();
                } else {
                    this->ws = ws;
                    this->_state = CONNECTED;
                    this->on_open();
                }
            });
            
            hub->uWS::Group<isServer>::onMessage([this](uWS::WebSocket<isServer> *ws, char *message, size_t length, uWS::OpCode opCode) {
                debug("Got message %s %d\n", message, (int)length);
                if (this->close_immediately) {
                    ws->close();
                }
                else {
                    this->on_message(message, length); 
                }
            });
            
            hub->uWS::Group<isServer>::onDisconnection([this](uWS::WebSocket<isServer> * ws, int code, char *message, size_t length) {
                debug("Closed %d %s\n", code, message ? message : "(null)");
                if (this->_state != INVALID) {
                    this->_state = DISCONNECTED;
                }
                if (!this->close_immediately) {
                    this->on_close(code, message, length);
                }
            });
            // Work around for MANYLINUX initialisation bug, explicitly initialise empty headers
            map<string,string> extra_headers = map<string,string>(); 
            this->_state = CONNECTING;
            hub->connect(uri, nullptr, extra_headers);
            return 0;
        }
        
        /**
         * Wrapper for python destructor
         */
        void __del__() {
            debug("Called __del__\n");
            if (this->connected()) {
                this->close();
            }
            delete this->hub;
            delete this->td;
            this->hub = NULL;
            this->td = NULL;
            this->ws = NULL;
            this->_state = INVALID;
        }
    private:
        PyObject * check_error(const char * name) {
            if (this->_state == INVALID) {
                PyErr_SetString(uWebSockets_error, "");
                this->_state = EXCEPTION;
                this->close();
                return NULL;
            }
            Py_RETURN_NONE;
        }
    public: 
        /**
         * Message handler
         */
        PyObject * on_message(char * message, size_t length) {
            debug("Got message %s %d\n", message, (int)length);
            PyGILState_STATE gstate = PyGILState_Ensure();
            PyObject * result = PyObject_CallMethod((PyObject*)this, (char*)"on_message", (char*)"s#", message, length);
            if (result == NULL) {
                fprintf(stderr, "Error in on_message(%s,%d)\n", message, (int)length);
                PyErr_WriteUnraisable((PyObject*)this);
                this->close();
                this->_state = INVALID;
            }
            PyGILState_Release(gstate);
            return result;
        }
        
        /**
         * Send a message
         */
        PyObject * send(char * message, size_t length) {
            debug("Sending: \"%s\" %d\n", message, (int)length);
            if (!this->ws || this->valid() == Py_False) {
                debug("Not connected!\n");
                PyErr_SetString(uWebSockets_error, "WebSocket not connected yet");
                return NULL;
            } else if (this->connected() == Py_False) {
                PyErr_SetString(uWebSockets_error, "WebSocket has disconnected");
                return NULL;
            }
            this->ws->send(message, length, uWS::OpCode::TEXT);
            return this->check_error("send");
        }
        

        
        PyObject * run(bool background) {
            debug("Called run %d\n", background);
            #ifndef _NO_THREADS
            if (background) {
                this->td = new thread([this]() {
                    this->hub->run();
                });
            } else 
            #endif //_NO_THREADS
            {
                Py_BEGIN_ALLOW_THREADS
                this->hub->run();
                Py_END_ALLOW_THREADS
            }
            return this->check_error("run");
        }
        
        PyObject * on_open() {
            debug("GIL state %d\n", PyGILState_Check());
            PyGILState_STATE gstate = PyGILState_Ensure();
            debug("GIL state %d\n", PyGILState_Check());
            PyObject * result = PyObject_CallMethod((PyObject*)this, (char*)"on_open", NULL); // NOTE: Ignore the -Wwrite-strings warning
            debug("GIL state %d\n", PyGILState_Check());
            debug("Result is %p , %s\n", (void*)result, (result == Py_True) ? "True" : (result == Py_False) ? "False" : "???");
            if (result == NULL) {
                fprintf(stderr, "Error in on_open()");
                PyErr_WriteUnraisable((PyObject*)this);
                this->close();
                this->_state = INVALID;
            }
            PyGILState_Release(gstate);
            return result;
        }
        
        PyObject * connected() {
            if (this->_state == INVALID) {
                return this->check_error("connected");
            }
            if (this->_state == CONNECTED) {
                Py_RETURN_TRUE;
            } else {
                Py_RETURN_FALSE;
            }
        }
        
        PyObject * valid() {
            if (this->_state == INVALID) {
                return this->check_error("valid");
            }
            if (this->_state == CONNECTED || this->_state == DISCONNECTED) {
                Py_RETURN_TRUE;
            } else {
                Py_RETURN_FALSE;
            }
        }
        
        PyObject * on_close(int code, char * message, int length) {
            if (this->close_immediately) {
                Py_RETURN_NONE;
            }
            PyGILState_STATE gstate = PyGILState_Ensure();
            PyObject * result = PyObject_CallMethod((PyObject*)this, (char*)"on_close", (char*)"is#", code, message, length); // NOTE: Ignore the -Wwrite-strings warning
            if (result == NULL) {
                fprintf(stderr, "Error in on_close(%d, %s,%d)\n", code, message, length);
                PyErr_WriteUnraisable((PyObject*)this);;
                this->_state = INVALID;
            }
            PyGILState_Release(gstate);
            //this->__del__();
            return result;
        }
        
        PyObject * close() {
            debug("Close requested\n");
            PyGILState_STATE gstate = PyGILState_Ensure();
            if (this->ws) {
                debug("Close socket\n");
                uWS::WebSocket<isServer> * w = this->ws;
                this->ws = NULL;
                w->close();
            } else {
                this->close_immediately = true;
            }
            #ifndef _NO_THREADS
            if (this->td && this_thread::get_id() != this->td->get_id()) {
                debug("Join thread\n");
                thread * t = td;
                this->td = NULL;
                t->join();
            }
            #endif
            PyGILState_Release(gstate);
            return this->check_error("close");
        }
        
        PyObject * force_close() {
            this->close_immediately = true;
            return this->close();
        }
        
        PyObject_HEAD
        
    private:
        // The websocket
        uWS::WebSocket<isServer> * ws;
        // The Hub
        uWS::Hub * hub;
        #ifndef _NO_THREADS
        thread * td;
        #else
        void * td;
        #endif
        enum {
            INVALID=0,
            CONNECTING=1,
            CONNECTED=2,
            DISCONNECTED=3,
            EXCEPTION=6,
        } _state;
        bool close_immediately;
        
};

template <bool isServer>
static PyObject * WebSocket_new(PyTypeObject * type, PyObject * args, PyObject * kwargs) {
    debug("Construct new object\n");
    WebSocket<isServer> * self = (WebSocket<isServer>*)(type->tp_alloc(type, 0));
    return (PyObject*)(self);
}

template <bool isServer>
static void WebSocket_dealloc(WebSocket<isServer> * self) {
    if (self != NULL) {
        self->__del__();
        Py_TYPE(self)->tp_free((PyObject*)self);
    }
}

template <bool isServer>
static PyObject * WebSocket_run(PyObject * self, PyObject * args) {
    bool background;
    if (!PyArg_ParseTuple(args, "b", &background)) {
        return NULL;
    }
    return ((WebSocket<isServer>*)(self))->run(background);
}

template <bool isServer>
static int WebSocket_init(WebSocket<isServer> * self, PyObject * args, PyObject * kwargs) {
    debug("Initialise new object\n");
    char * uri;
    int length;
    if (!PyArg_ParseTuple(args, "s#", &uri, &length)) {
        return 1;
    }
    debug("URI: %s, %d\n", uri, length);
    return self->__init__(uri);
}

template <bool isServer>
static PyObject * WebSocket_on_message(PyObject * self, PyObject * args) {
    char * message;
    int length;
    if (!PyArg_ParseTuple(args, "s#", &message, &length)) {
        return NULL;
    }
    return ((WebSocket<isServer>*)(self))->on_message(message, length);
}

template <bool isServer>
static PyObject * WebSocket_send(PyObject * self, PyObject * args) {
    char * message;
    int length;
    debug("Sending...\n");
    if (!PyArg_ParseTuple(args, "s#", &message, &length)) {
        return NULL;
    }
    debug("Args: %s %d\n", message, length);
    return ((WebSocket<isServer>*)(self))->send(message, length);
}

template <bool isServer>
static PyObject * WebSocket_close(PyObject * self, PyObject * args) {
    return ((WebSocket<isServer>*)(self))->close();
}

template <bool isServer>
static PyObject * WebSocket_connected(PyObject * self, PyObject * args) {
    return ((WebSocket<isServer>*)(self))->connected();
}

template <bool isServer>
static PyObject * WebSocket_valid(PyObject * self, PyObject * args) {
    return ((WebSocket<isServer>*)(self))->valid();
}

template <bool isServer>
static PyObject * WebSocket_getattr(PyObject * self, PyObject * name) {
    #if (PYTHON_FLAVOUR == 3)
    char * cname = PyBytes_AsString(PyUnicode_AsASCIIString(name));
    #else
    char * cname = PyString_AsString(name);
    #endif
    if (cname == NULL) {
        return NULL;
    }
    PyObject * result = PyObject_GenericGetAttr(self, name);
    if (result == NULL) {
        fprintf(stderr, "No such attribute \"%s\"\n", cname);
        WebSocket<isServer> * w = (WebSocket<isServer>*)(self);
        w->force_close();
    }
    return result;
}

static PyMethodDef WebSocketClient_methods[] = {
    {"on_message", WebSocket_on_message<false>, METH_VARARGS, (char*)"Callback for receiving a message"},
    {"send", WebSocket_send<false>, METH_VARARGS, (char*)"Send a message to connected peer"},
    {"run", WebSocket_run<false>, METH_VARARGS, (char*)"Start the event loop"},
    {"close", WebSocket_close<false>, METH_VARARGS, (char*)"Close the WebSocket"},
    {"connected", WebSocket_connected<false>, METH_VARARGS, (char*)"Is the WebSocket currently connected right now?"},
    {"valid", WebSocket_valid<false>, METH_VARARGS, (char*)"Is the WebSocket valid?"},
    {NULL}
};



/**
 * Python Type Object for the WebSocket class
 */
static PyTypeObject uWebSockets_WebSocketClientType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "uWebSockets.WebSocketClient", /* tp_name */
    sizeof(WebSocket<false>),  /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)WebSocket_dealloc<false>,                         /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    WebSocket_getattr<false>,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /* tp_flags */
    "A WebSocket Client",      /* tp_doc */
    0,
    0,
    0,
    0,
    0,
    0,
    WebSocketClient_methods,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    (initproc)WebSocket_init<false>,
    0,
    WebSocket_new<false>,
};


/**
 * Public methods
 */
static PyMethodDef uWebSockets_methods[] = {
    {"hello", uWebSockets_hello, METH_VARARGS, "Prints 'Hello, World'"},
    //{"WebSocketClient", , METH_VARARGS, "Create WebSocket client"},
    {NULL, NULL, 0, NULL}
};



#if (PYTHON_FLAVOUR == 3)
/**
 * Module definition structure
 * (Py_InitModule is Python2 only)
 */
static struct PyModuleDef uWebSockets_module_def = {
    PyModuleDef_HEAD_INIT,
    "uWebSockets", // name
    "Python bindings for the uWebSockets library", // docstring
    -1, // Keep state in globals
    uWebSockets_methods //module methods
};
#endif

/**
 * Module initialisation function 
 */
PyMODINIT_FUNC
inituWebSockets(void) {
    debug("Called inituWebSockets\n");   
    if (!Py_IsInitialized()) { // If for some reason we are invoking the interpreter from here rather than the other way around...
        debug("Initialise");
        Py_Initialize();
    } 
    PyObject * m = NULL;
    #if (PYTHON_FLAVOUR == 3)
        m = PyModule_Create(&uWebSockets_module_def); // Python3 way
    #else
        m = Py_InitModule("uWebSockets", uWebSockets_methods); // Python2 way
    #endif
    
    if (m == NULL) {
        #if (PYTHON_FLAVOUR == 3)
            Py_RETURN_NONE;
        #else
            return;
        #endif
    }
    
    uWebSockets_error = PyErr_NewException((char*)"uWebSockets.Error", NULL, NULL); 
    Py_INCREF(uWebSockets_error);
    PyModule_AddObject(m, "Error", uWebSockets_error);
    if (PyType_Ready(&uWebSockets_WebSocketClientType) < 0) {
        fprintf(stderr, __FILE__":Failed to construct WebSocketClientType\n");
        #if (PYTHON_FLAVOUR == 3)
            return m;
        #else
            return;
        #endif
    }
    Py_INCREF(&uWebSockets_WebSocketClientType);
    PyModule_AddObject(m, "WebSocketClient", (PyObject*)&uWebSockets_WebSocketClientType);
    
    PyEval_InitThreads();
    #if (PYTHON_FLAVOUR == 3)
        return m;
    #endif
}

// Define both PyInit_uWebSockets and inituWebSockets for compatibility 
PyMODINIT_FUNC
PyInit_uWebSockets(void) {
    debug("Called PyInit_uWebSockets\n");
    #if (PYTHON_FLAVOUR == 3)
    return inituWebSockets();
    #else
    inituWebSockets();
    #endif
}

#ifdef MANYLINUX
typedef void (*_exitfn)(void);
static vector<_exitfn> _atexit = vector<_exitfn>();

void __wrap__fini(void) {
    //TODO: Remove this horrible hack
    for (_exitfn & fn : _atexit) {
        (*fn)();
    }
}

extern "C" {
    void __wrap_atexit(void(*function)(void)) {
        _atexit.push_back(function);
    }
}
#endif
