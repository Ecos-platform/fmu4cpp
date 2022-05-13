import sys
from ctypes import *

if __name__ == "__main__":
    dll = cdll.LoadLibrary(sys.argv[1])
    dll.write_description(b"modelDescription.xml")


