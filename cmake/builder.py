import sys
import zipfile
import platform
from ctypes import *
from pathlib import Path


def get_lib_extension() -> str:
    """Get FMU library platform extension."""
    platforms = {"Darwin": "dylib", "Linux": "so", "Windows": "dll"}
    return platforms.get(platform.system(), "")


def get_platform() -> str:
    """Get FMU binary platform folder name."""
    system = platform.system()
    is_64bits = sys.maxsize > 2 ** 32
    platforms = {"Windows": "win", "Linux": "linux", "Darwin": "darwin"}
    return platforms.get(system, "unknown") + "64" if is_64bits else "32"


if __name__ == "__main__":
    modelIdentifier = sys.argv[1]
    dllPath = sys.argv[2]
    dll = cdll.LoadLibrary(dllPath)
    dll.write_description(b"modelDescription.xml")

    dest_file = f"{modelIdentifier}.fmu"
    with zipfile.ZipFile(dest_file, "w") as zip_fmu:
        binaries = Path("binaries")
        zip_fmu.write(dllPath, arcname=(binaries / get_platform() / f"{modelIdentifier}.{get_lib_extension()}"))
        zip_fmu.write("modelDescription.xml")
