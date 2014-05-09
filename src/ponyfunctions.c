#include "ponyfunctions.h"

#include <Python.h>

#define LUALIB
#include "lua.h"
#include "lauxlib.h"

static PyObject *convert_dict_lua_to_python(lua_State *L, int table_index) {
    luaL_checktype(L, table_index, LUA_TTABLE);

    PyObject *dict = PyDict_New();
    if (!dict) {
        PyErr_Print();
        fprintf(stderr, "Failed to create dictionary\n");
        return NULL;
    }

    lua_pushnil(L);
    while (lua_next(L, table_index)) {
        const char *key_string = luaL_checkstring(L, -2);
        PyObject *key = PyString_FromString(key_string);

        PyObject *value;
        switch (lua_type(L, -1)) {
        case LUA_TSTRING:
            value = PyString_FromString(luaL_checkstring(L, -1));
            break;
        case LUA_TNUMBER:
            value = PyInt_FromLong(luaL_checkinteger(L, -1));
            break;
        case LUA_TBOOLEAN:
            value = PyBool_FromLong(lua_toboolean(L, -1));
            break;
        default:
            luaL_error(L, "unsupported type");
            return NULL;
        }

        PyDict_SetItem(dict, key, value);

        Py_DECREF(key);
        Py_DECREF(value);

        lua_pop(L, 1);
    }

    return dict;
}

static int convert_dict_python_to_lua(lua_State *L, PyObject *dict) {
    PyObject *items = PyMapping_Items(dict);
    if (!items) {
        PyErr_Print();
        fprintf(stderr, "Failed to get items\n");
        return -1;
    }

    lua_newtable(L);

    for (Py_ssize_t i = 0; i < PyList_Size(items); i++) {
        PyObject *item = PyList_GetItem(items, i);
        if (!item) {
            PyErr_Print();
            fprintf(stderr, "Failed to get item\n");
            return -1;
        }

        PyObject *key = PyTuple_GetItem(item, 0);
        if (!key) {
            PyErr_Print();
            fprintf(stderr, "Failed to get key\n");
            return -1;
        }
        const char *key_string = PyString_AsString(key);
        if (!key_string) {
            PyErr_Print();
            fprintf(stderr, "Failed to get key string\n");
            return -1;
        }

        PyObject *value = PyTuple_GetItem(item, 1);
        if (!value) {
            PyErr_Print();
            fprintf(stderr, "Failed to get value\n");
            return -1;
        }
        if (PyString_Check(value)) {
            const char *value_string = PyString_AsString(value);
            if (!value_string) {
                PyErr_Print();
                fprintf(stderr, "Failed to get value string\n");
                return -1;
            }
            lua_pushstring(L, value_string);
        } else if (PyInt_Check(value)) {
            long value_num = PyInt_AsLong(value);
            if (value_num == -1 && PyErr_Occurred()) {
                PyErr_Print();
                fprintf(stderr, "Failed to get value integer\n");
                return -1;
            }
            lua_pushinteger(L, value_num);
        }

        lua_setfield(L, -2, key_string);

        Py_DECREF(key);
        Py_DECREF(value);
        Py_DECREF(item);
    }

    Py_DECREF(items);

    return 0;
}

static PyObject *load_pony_function(const char *name) {
    PyObject *sys = PyImport_ImportModule("sys");
    PyObject *path = PyObject_GetAttrString(sys, "path");
    PyList_Append(path, PyString_FromString("python"));

    PyObject *module_name = PyString_FromString("ponyfunctions");
    PyObject *module = PyImport_Import(module_name);
    Py_DECREF(module_name);

    if (module == NULL) {
        PyErr_Print();
        fprintf(stderr, "Failed to load ponyfunctions\n");
        return NULL;
    }

    PyObject *class = PyObject_GetAttrString(module, "PonyFunctions");
    if (!class) {
        PyErr_Print();
        fprintf(stderr, "Cannot find class PonyFunctions.\n");
        return NULL;
    }
    Py_DECREF(module);
    PyObject *func = PyObject_GetAttrString(class, name);
    if (!func) {
        PyErr_Print();
        fprintf(stderr, "Cannot find function '%s'\n", name);
        return NULL;
    }
    if (!PyCallable_Check(func)) {
        fprintf(stderr, "Function '%s' not executable\n", name);
        return NULL;
    }

    return func;
}

static int call_pony_function(lua_State *L,
                              PyObject *func,
                              PyObject *args,
                              PyObject *kwargs) {
    PyObject *result = PyObject_Call(func, args, kwargs);
    Py_DECREF(args);
    Py_DECREF(func);
    if (!result) {
        PyErr_Print();
        fprintf(stderr, "Call failed\n");
        lua_pushnil(L);
        lua_pushstring(L, "error calling pony_dns function");
        Py_Finalize();
        return 2;
    }
    if (convert_dict_python_to_lua(L, result)) {
        fprintf(stderr, "Conversion failed\n");
        lua_pushnil(L);
        lua_pushstring(L, "error converting pony_dns results");
        Py_Finalize();
        return 2;
    }
    lua_pushnil(L);

    Py_Finalize();

    return 2;
}

int pony_ping(lua_State *L) {
    Py_Initialize();

    PyObject *func = load_pony_function("pony_ping");
    if (!func) {
        lua_pushnil(L);
        lua_pushstring(L, "error loading pony_ping function");
        return 2;
    }

    const char *output_string = luaL_checkstring(L, 1);
    PyObject *output = PyString_FromString(output_string);
    const char *ip_string = luaL_checkstring(L, 2);
    PyObject *ip = PyString_FromString(ip_string);
    PyObject *kwargs = convert_dict_lua_to_python(L, 3);

    PyObject *args = PyTuple_New(2);
    PyTuple_SetItem(args, 0, output);
    PyTuple_SetItem(args, 1, ip);

    return call_pony_function(L, func, args, kwargs);
}

int pony_traceroute(lua_State *L) {
    Py_Initialize();

    PyObject *func = load_pony_function("pony_traceroute");
    if (!func) {
        lua_pushnil(L);
        lua_pushstring(L, "error loading pony_traceroute function");
        return 2;
    }

    const char *output_string = luaL_checkstring(L, 1);
    PyObject *output = PyString_FromString(output_string);
    const char *ip_string = luaL_checkstring(L, 2);
    PyObject *ip = PyString_FromString(ip_string);
    PyObject *kwargs = convert_dict_lua_to_python(L, 3);

    PyObject *args = PyTuple_New(2);
    PyTuple_SetItem(args, 0, output);
    PyTuple_SetItem(args, 1, ip);

    return call_pony_function(L, func, args, kwargs);
}

int pony_fasttraceroute(lua_State *L) {
    Py_Initialize();

    PyObject *func = load_pony_function("pony_fasttraceroute");
    if (!func) {
        lua_pushnil(L);
        lua_pushstring(L, "error loading pony_fasttraceroute function");
        return 2;
    }

    const char *output_string = luaL_checkstring(L, 1);
    PyObject *output = PyString_FromString(output_string);
    const char *ip_string = luaL_checkstring(L, 2);
    PyObject *ip = PyString_FromString(ip_string);
    PyObject *kwargs = convert_dict_lua_to_python(L, 3);

    PyObject *args = PyTuple_New(2);
    PyTuple_SetItem(args, 0, output);
    PyTuple_SetItem(args, 1, ip);

    return call_pony_function(L, func, args, kwargs);
}

int pony_dns(lua_State *L) {
    Py_Initialize();

    PyObject *func = load_pony_function("pony_dns");
    if (!func) {
        lua_pushnil(L);
        lua_pushstring(L, "error loading pony_dns function");
        return 2;
    }

    const char *output_string = luaL_checkstring(L, 1);
    PyObject *output = PyString_FromString(output_string);
    const char *domain_string = luaL_checkstring(L, 2);
    PyObject *domain = PyString_FromString(domain_string);
    PyObject *kwargs = convert_dict_lua_to_python(L, 3);

    PyObject *args = PyTuple_New(2);
    PyTuple_SetItem(args, 0, output);
    PyTuple_SetItem(args, 1, domain);

    return call_pony_function(L, func, args, kwargs);
}

int pony_gethttp(lua_State *L) {
    Py_Initialize();

    PyObject *func = load_pony_function("pony_gethttp");
    if (!func) {
        lua_pushnil(L);
        lua_pushstring(L, "error loading pony_gethttp function");
        Py_Finalize();
        return 2;
    }

    const char *output_string = luaL_checkstring(L, 1);
    PyObject *output = PyString_FromString(output_string);
    const char *scheme_string = luaL_checkstring(L, 2);
    PyObject *scheme = PyString_FromString(scheme_string);
    const char *domain_string = luaL_checkstring(L, 3);
    PyObject *domain = PyString_FromString(domain_string);
    const char *path_string = luaL_checkstring(L, 4);
    PyObject *path = PyString_FromString(path_string);
    const char *host_string = luaL_checkstring(L, 5);
    PyObject *host = PyString_FromString(host_string);
    PyObject *kwargs = convert_dict_lua_to_python(L, 6);

    PyObject *args = PyTuple_New(5);
    PyTuple_SetItem(args, 0, output);
    PyTuple_SetItem(args, 1, scheme);
    PyTuple_SetItem(args, 2, domain);
    PyTuple_SetItem(args, 3, path);
    PyTuple_SetItem(args, 4, host);

    return call_pony_function(L, func, args, kwargs);
}
