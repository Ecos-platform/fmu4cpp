import ctypes
from ctypes import *

if __name__ == "__main__":
    dll = cdll.LoadLibrary("../cmake-build-debug/examples/libsine")
    dll.fmi2GetTypesPlatform.restype = ctypes.c_char_p
    types = dll.fmi2GetTypesPlatform()
    print(types)

