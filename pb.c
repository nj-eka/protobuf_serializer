#define PY_SSIZE_T_CLEAN  # https://docs.python.org/3.9/c-api/intro.html
#include <Python.h>
#include <stdint.h>
#include <zlib.h>

#include "deviceapps.pb-c.h"

#define MAGIC  0xFFFFFFFF
#define DEVICE_APPS_TYPE 1
#define PBHEADER_INIT {MAGIC, 0, 0}

typedef struct pbheader_s {
    uint32_t magic;
    uint16_t type;
    uint16_t length;
} pbheader_t;

size_t device_apps_serialize(PyObject* py_item, gzFile zfile) {
    // example: py_item = {"device": {"type": "gaid", "id": "e7e1a50c0ec2747ca56cd9e1558c0d7d"}, "lat": 42, "lon": -42, "apps": [1, 2]}

    // message DeviceApps {
    //     message Device {
    //         optional bytes id = 1;
    //         optional bytes type = 2;
    //     }
    //     optional Device device = 1;
    //     repeated uint32 apps = 2;
    //     optional double lat = 3;
    //     optional double lon = 4;
    // }
    DeviceApps pbf_device_apps = DEVICE_APPS__INIT;
    DeviceApps__Device pbf_device = DEVICE_APPS__DEVICE__INIT;
    pbf_device_apps.device = &pbf_device;

    // optional Device device = 1;
    PyObject* py_device = PyDict_GetItemString(py_item, "device");
    if (py_device) {
        if (!PyDict_Check(py_device)) {
            PyErr_Format(PyExc_TypeError,
                        "[device] element must be a dictionary not a '%s'",
                        Py_TYPE(py_device)->tp_name);            
            return -1;
        }

        // optional bytes id = 1;
        PyObject* py_device_id = PyDict_GetItemString(py_device, "id");
        if (py_device_id) {
            Py_ssize_t len = 0;
            const char* device_id = PyUnicode_AsUTF8AndSize(py_device_id, &len);
            if (PyErr_Occurred()) {
                PyErr_Format(PyExc_TypeError,
                            "[device.id] element must be a string not a '%s'",
                            Py_TYPE(py_device_id)->tp_name);            
                return -1;
            } 
            pbf_device_apps.device->has_id = 1;
            pbf_device_apps.device->id.data = (uint8_t*)device_id;
            pbf_device_apps.device->id.len = len;
        }
        
        // optional bytes type = 2;
        PyObject* py_device_type = PyDict_GetItemString(py_device, "type");
        if (py_device_type) {
            Py_ssize_t len = 0;
            const char* device_type = PyUnicode_AsUTF8AndSize(py_device_type, &len);
            if (PyErr_Occurred()) {
                PyErr_Format(PyExc_TypeError,
                            "[device.type] element must be a string not a '%s'",
                            Py_TYPE(py_device_type)->tp_name);            
                return -1;
            }            
            pbf_device_apps.device->has_type = 1;
            pbf_device_apps.device->type.data = (uint8_t*)device_type;
            pbf_device_apps.device->type.len = len;
        }
    }

    // repeated uint32 apps = 2;
    // *refs repeated: this field can be repeated any number of times (INCLUDING ZERO) in a well-formed message. The order of the repeated values will be preserved.
    PyObject* py_apps = PyDict_GetItemString(py_item, "apps");
    if (py_apps) {
        if (!PyList_Check(py_apps)) {
            PyErr_Format(PyExc_TypeError,
                        "[apps] element must be a list not a '%s'",
                        Py_TYPE(py_apps)->tp_name);            
            return -1;
        }
        size_t apps_size = PyList_Size(py_apps);
        if (apps_size){
            pbf_device_apps.n_apps = apps_size;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            pbf_device_apps.apps = malloc(sizeof(size_t) * apps_size);          
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if (!pbf_device_apps.apps) {
                PyErr_SetString(PyExc_MemoryError, "Memory error.");
                return -1;
            }
            for (size_t i = 0; i < apps_size; i++) {
                PyObject* py_app = PyList_GET_ITEM(py_apps, i);
                if ((py_app == NULL) || !PyLong_Check(py_app)) {
                    PyErr_Format(PyExc_TypeError,
                                "[app] element must be a int not a '%s'",
                                Py_TYPE(py_app)->tp_name);            
                    free(pbf_device_apps.apps);
                    return -1;
                }
                pbf_device_apps.apps[i] = (uint32_t)PyLong_AsLong(py_app);
            }
        }
    }

    // optional double lat = 3;
    PyObject* py_lat = PyDict_GetItemString(py_item, "lat");
    if (py_lat) {
        pbf_device_apps.lat = PyFloat_AsDouble(py_lat);
        if (PyErr_Occurred() != NULL) {
            PyErr_SetString(PyExc_TypeError, "[lat] isn't a number.");
            free(pbf_device_apps.apps);
            return -1;
        }         
        pbf_device_apps.has_lat = 1;
    }
    // optional double lon = 4;
    PyObject* py_lon = PyDict_GetItemString(py_item, "lon");
    if (py_lon) {
        pbf_device_apps.lon = PyFloat_AsDouble(py_lon);
        if (PyErr_Occurred() != NULL) {
            PyErr_SetString(PyExc_TypeError, "[lon] isn't a number.");
            free(pbf_device_apps.apps);
            return -1;
        }         
        pbf_device_apps.has_lon = 1;
    }
    size_t device_apps_packed_size = device_apps__get_packed_size(&pbf_device_apps);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void* device_apps_buffer = malloc(device_apps_packed_size);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (!device_apps_buffer) {
        PyErr_SetString(PyExc_MemoryError, "Memory Error.");
        free(pbf_device_apps.apps);
        return -1;
    }
    device_apps__pack(&pbf_device_apps, device_apps_buffer);

    pbheader_t pbheader = PBHEADER_INIT;
    pbheader.type = DEVICE_APPS_TYPE;
    pbheader.length = device_apps_packed_size;

    int pbheader_bytes_written = gzwrite(zfile, &pbheader, sizeof(pbheader_t));
    int device_apps_bytes_written = gzwrite(zfile, device_apps_buffer, (unsigned int)device_apps_packed_size);

    free(pbf_device_apps.apps);
    free(device_apps_buffer);

    if ( (pbheader_bytes_written != sizeof(pbheader_t)) || (device_apps_bytes_written != (int)device_apps_packed_size) ) {
        PyErr_SetString(PyExc_OSError, "Serialization failed.");
        return -1;
    }
    return pbheader_bytes_written + device_apps_bytes_written;
}


// Read iterator of Python dicts
// Pack them to DeviceApps protobuf and write to file with appropriate header
// Return number of written bytes as Python integer
static PyObject* py_deviceapps_xwrite_pb(PyObject* self, PyObject* args) {
    PyObject* obj; /* iterable object iter(obj) / __iter__ / PyObject_GetIter(obj)   */
    const char* fname; /* output file name */    

    if (!PyArg_ParseTuple(args, "Os", &obj, &fname))
        return NULL;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    gzFile zfile = gzopen(fname, "wb");
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (!zfile) {
        PyErr_Format(PyExc_OSError, "gzopen of '%s' failed.", fname);
        return NULL;
    }

    //* https://docs.python.org/3/c-api/object.html
    // This is equivalent to the Python expression iter(o). 
    // It returns a new iterator for the object argument, or the object itself if the object is already an iterator. 
    // Raises TypeError and returns NULL if the object cannot be iterated.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    PyObject* py_iter = PyObject_GetIter(obj);  // https://docs.python.org/3/c-api/iter.html#c.PyIter_Check
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (py_iter == NULL) {
        PyErr_SetString(PyExc_TypeError, "First argument must be Iterable.");
        gzclose(zfile);
        return NULL;
    }
    size_t total_bytes = 0;
    PyObject* py_item;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    while ((py_item = PyIter_Next(py_iter))) {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (PyDict_Check(py_item)) {
            size_t processed = device_apps_serialize(py_item, zfile);
            if (processed < 0) {
                Py_DECREF(py_item);
                Py_DECREF(py_iter);
                gzclose(zfile);
                return NULL;
            } 
            total_bytes += processed;
        }
        // else { terms of task does not specify what to do in this case. so let's continue. }
        Py_DECREF(py_item);
    }    
    Py_DECREF(py_iter);
    gzclose(zfile);
    return PyLong_FromSize_t(total_bytes);  
}

PyObject* deserialize(DeviceApps* pbf_device_apps) {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    PyObject* py_device_apps = PyDict_New();
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (py_device_apps == NULL) {            
        PyErr_SetFromErrno(PyExc_RuntimeError);      
        goto error;
    }

    // optional Device device = 1;    
    if (pbf_device_apps->device) {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        PyObject* py_device = PyDict_New();
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (py_device == NULL) {
            PyErr_SetFromErrno(PyExc_RuntimeError);
            goto error;
        }

        // optional bytes id = 1;
        if (pbf_device_apps->device->has_id) {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            PyObject* py_value = Py_BuildValue("s#", pbf_device_apps->device->id.data, pbf_device_apps->device->id.len);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if (py_value == NULL) {
                PyErr_SetFromErrno(PyExc_RuntimeError);   
                Py_DECREF(py_device);
                goto error;
            }
            PyDict_SetItemString(py_device, "id", py_value);
            Py_DECREF(py_value);
            if (PyErr_Occurred()) {            
                PyErr_SetFromErrno(PyExc_RuntimeError);   
                Py_DECREF(py_device);
                goto error;
            }
        }

        // optional bytes type = 2;
        if (pbf_device_apps->device->has_type) {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            PyObject* py_value = Py_BuildValue("s#", pbf_device_apps->device->type.data, pbf_device_apps->device->type.len);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if (py_value == NULL) {
                PyErr_SetFromErrno(PyExc_RuntimeError);   
                Py_DECREF(py_device);
                goto error;
            }
            PyDict_SetItemString(py_device, "type", py_value);
            Py_DECREF(py_value);
            if (PyErr_Occurred()) {            
                PyErr_SetFromErrno(PyExc_RuntimeError);   
                Py_DECREF(py_device);
                goto error;
            }
        }
        PyDict_SetItemString(py_device_apps, "device", py_device);
        Py_DECREF(py_device);
        if (PyErr_Occurred()) {            
            PyErr_SetFromErrno(PyExc_RuntimeError);   
            goto error;
        }

    }

    // repeated uint32 apps = 2;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    PyObject* py_apps = PyList_New(0);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (py_apps == NULL) {            
        PyErr_SetFromErrno(PyExc_RuntimeError);        
        goto error;
    }    
    if (pbf_device_apps->n_apps) {
        for (size_t i = 0; i < pbf_device_apps->n_apps; i++) {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            PyObject* py_value = PyLong_FromLong(pbf_device_apps->apps[i]);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if (py_value == NULL) {            
                PyErr_SetFromErrno(PyExc_RuntimeError);        
                Py_DECREF(py_apps);
                goto error;
            }
            PyList_Append(py_apps, py_value);
            Py_DECREF(py_value);
            if (PyErr_Occurred()) {            
                PyErr_SetFromErrno(PyExc_RuntimeError);        
                Py_DECREF(py_apps);
                goto error;
            }
        }
    }
    PyDict_SetItemString(py_device_apps, "apps", py_apps);
    Py_DECREF(py_apps);

    // optional double lat = 3;
    if (pbf_device_apps->has_lat) {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        PyObject* py_value = PyFloat_FromDouble(pbf_device_apps->lat);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (py_value == NULL) {            
            PyErr_SetFromErrno(PyExc_RuntimeError);      
            goto error;
        }
        PyDict_SetItemString(py_device_apps, "lat", py_value);
        Py_DECREF(py_value);
        if (PyErr_Occurred()) {            
            PyErr_SetFromErrno(PyExc_RuntimeError);        
            goto error;
        }
    }

    // optional double lon = 4;
    if (pbf_device_apps->has_lon) {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        PyObject* py_value = PyFloat_FromDouble(pbf_device_apps->lon);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (py_value == NULL) {            
            PyErr_SetFromErrno(PyExc_RuntimeError);      
            goto error;
        }
        PyDict_SetItemString(py_device_apps, "lon", py_value);
        Py_DECREF(py_value);
        if (PyErr_Occurred()) {
            PyErr_SetFromErrno(PyExc_RuntimeError);        
            goto error;
        }
    }

    return py_device_apps;

error:
    Py_XDECREF(py_device_apps);
    return NULL;
}


// Unpack only messages with type == DEVICE_APPS_TYPE
// Return iterator of Python dicts
static PyObject* py_deviceapps_xread_pb(PyObject* self, PyObject* args) {
    const char* fname;
    if (!PyArg_ParseTuple(args, "s", &fname))
        return NULL;

    if (access(fname, F_OK) == -1 ) {
        PyErr_Format(PyExc_OSError, "No such file: %s", fname);
        return NULL;
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    PyObject* py_list = PyList_New(0);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (py_list == NULL) {
        PyErr_SetFromErrno(PyExc_RuntimeError);
        return NULL;
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    gzFile zfile = gzopen(fname, "rb");
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (zfile == NULL) {
        PyErr_Format(PyExc_OSError, "gzopen of '%s' failed.", fname);
        Py_DECREF(py_list);        
        return NULL;
    }

    while (!gzeof(zfile)) {
        pbheader_t pbheader;
        size_t bytes_read = gzread(zfile, &pbheader, sizeof(pbheader_t));
        if (bytes_read) {
            if ((bytes_read != sizeof(pbheader_t)) || (pbheader.magic != MAGIC)) {
                PyErr_SetString(PyExc_ValueError, "Wrong file format.");
                goto free_res; 
            }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            void* device_apps_buffer = malloc(pbheader.length);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if (!device_apps_buffer) {
                PyErr_SetString(PyExc_MemoryError, "Memory error.");
                goto free_res;
            }
            bytes_read = gzread(zfile, device_apps_buffer, pbheader.length);
            if (bytes_read != pbheader.length){
                PyErr_SetString(PyExc_ValueError, "Wrong file format.");
                free(device_apps_buffer);
                goto free_res;             
            }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            DeviceApps* msg = device_apps__unpack(NULL, pbheader.length, device_apps_buffer);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if (!msg){
                PyErr_SetFromErrno(PyExc_RuntimeError);        
                free(device_apps_buffer);
                goto free_res;             
            }
            PyObject* py_msg = deserialize(msg);
            if (py_msg == NULL) {
                free(device_apps_buffer);
                device_apps__free_unpacked(msg, NULL);
                goto free_res;             
            }
            PyList_Append(py_list, py_msg);
            Py_DECREF(py_msg);
            free(device_apps_buffer);
            device_apps__free_unpacked(msg, NULL);
        } else
            break;
    }

free_res:
    gzclose(zfile);
    if (PyErr_Occurred()) {
        Py_DECREF(py_list);
        return NULL;
    }
    PyObject* py_iter = PySeqIter_New(py_list);
    Py_DECREF(py_list);
    return py_iter;
}


static PyMethodDef PBMethods[] = {
     {"deviceapps_xwrite_pb", py_deviceapps_xwrite_pb, METH_VARARGS, "Write serialized protobuf to file fro iterator"},
     {"deviceapps_xread_pb", py_deviceapps_xread_pb, METH_VARARGS, "Deserialize protobuf from file, return iterator"},
     {NULL, NULL, 0, NULL}
};

static PyModuleDef PBModule = {
    PyModuleDef_HEAD_INIT, "pb", "Protobuf (de)serializer", -1, PBMethods
};

PyMODINIT_FUNC PyInit_pb(void) {
    return PyModule_Create(&PBModule);
}