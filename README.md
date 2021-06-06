# Protobuf (de)serializer
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
    - python3-pip
    - python3-dev
    - python3-setuptools  
    - zlib1g-dev

- Python libraries: 
    - protobuf>=3.17

## Instalation and usage:
- Common steps:
```
$ git clone https://github.com/nj-eka/protobuf_serializer.git
$ cd protobuf_serializer
```
- Run on host: 
    - Just run test:

        *it is assumed that all necessary libraries and programs are already installed on the system*
        ```
        $ pip install -r requirements.txt
        $ pip instal .
        $ python setup.py test
        ```
    - Full path - Install, compile and run test:
        ```
        $ sudo apt-get update -qq
        $ sudo apt-get install -yq --no-install-recommends \
            gcc gdb make \
            zlib1g-dev \
            protobuf-c-compiler libprotobuf-c-dev \
            python3 python3-pip python3-dev python3-setuptools
        $ sudo apt-get clean
        $ pip install -r requirements.txt
        $ protoc-c --c_out=. deviceapps.proto
        $ pip install .
        $ protoc --python_out=. deviceapps.proto
        $ python setup.py test
        ```



- Run in docker:
    ```
    $ docker build --rm -t pbc .
    ...
    $ docker run -it --rm -v $PWD:/opt/protobuf_serializer pbc
    root@465da45484c6:/opt/protobuf_serializer# bash start.sh 
    ...
    test_read (tests.test_pb.TestPB) ... ok
    test_write (tests.test_pb.TestPB) ... ok

    ----------------------------------------------
    Ran 2 tests in 0.003s

    OK    

    ```


## Useful links:
- [Protocol Buffers](https://developers.google.com/protocol-buffers/docs/proto)
- [Coding Patterns for Python Extensions](https://pythonextensionpatterns.readthedocs.io/en/latest/index.html)
- [Implementing a generator/yield in a Python C extension](https://eli.thegreenplace.net/2012/04/05/implementing-a-generatoryield-in-a-python-c-extension)
- [Porting guide for Python C Extensions](https://py3c.readthedocs.io/en/latest/guide.html)
- [Protobuf-c/wiki/Examples](https://github.com/protobuf-c/protobuf-c/wiki/Examples)
- [Building C and C++ Extensions](https://docs.python.org/3/extending/building.html)
- [Python/C API Reference Manual](https://docs.python.org/3.9/c-api/index.html)



