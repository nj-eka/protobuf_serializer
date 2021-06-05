# protobuf_serializer
*Protobuf (de)serializer*
## Purpose:
Make C extension module for serializing and deserializing protobuf's messages.

**To do:**

Complete C Extension `pb.c`:

- Implement serializer `deviceapps_xwrite_pb` that should:
    - Accept as input arguments: 
        - iterable object with dictionaries
        - file path for output 
    - Serialize dictionaries into protobuf's message and write gzipped into file
    - Prepend each protobuf's message with `pbheader_t` header
    - Return number of written bytes
- Implement deserializer `deviceapps_xread_pb` that should:
    - Accept as input argument:
        - path (to gzipped protobuf's messages file) 
    - Read these messages from `deviceapps_xwrite_pb` and convert its back to dictionaries
    - Return generator object with dictionaries
- Complete test suites accordingly.

## Requirements:
- OS: 
    - Linux: Ubuntu 20.* (CentOS 7/8)
- Depends: 
    - gcc
    - make
    - gdb https://www.gnu.org/software/gdb/
    - protobuf-c-compiler
    - libprotobuf-c-dev
    - python3.8+
    - python3-dev
    - python3-setuptools  
    - zlib1g-dev

- Python libraries: 
    - protobuf>=3.17

## Useful links:
- [Protocol Buffers](https://developers.google.com/protocol-buffers/docs/proto)
- [Coding Patterns for Python Extensions](https://pythonextensionpatterns.readthedocs.io/en/latest/index.html)
- [Implementing a generator/yield in a Python C extension](https://eli.thegreenplace.net/2012/04/05/implementing-a-generatoryield-in-a-python-c-extension)
- [Porting guide for Python C Extensions](https://py3c.readthedocs.io/en/latest/guide.html)
- [Protobuf-c/wiki/Examples](https://github.com/protobuf-c/protobuf-c/wiki/Examples)
- [Building C and C++ Extensions](https://docs.python.org/3/extending/building.html)
- [Python/C API Reference Manual](https://docs.python.org/3.9/c-api/index.html)



