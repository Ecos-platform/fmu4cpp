//this program is used internally to extract data from the model to populate the modelDescription.xml
//invoked during CMake configuration

#if defined(_MSC_VER) || defined(WIN32) || defined(__MINGW32__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define DLL_HANDLE HMODULE
#else
#define DLL_HANDLE void *
#include <dlfcn.h>
#endif

#ifdef WIN32
#include <sstream>
#define function_ptr FARPROC
#else
typedef void *function_ptr;
#endif

#include <filesystem>
#include <iostream>

namespace {

    DLL_HANDLE load_library(const std::string &libName) {
#ifdef WIN32
        return LoadLibrary(libName.c_str());
#else
        return dlopen(libName.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
    }

    template<class T>
    T load_function(DLL_HANDLE handle, const char *function_name) {
#ifdef WIN32
        return (T) GetProcAddress(handle, function_name);
#else
        return (T) dlsym(handle, function_name);
#endif
    }

    bool free_library(DLL_HANDLE handle) {
#ifdef WIN32
        return static_cast<bool>(FreeLibrary(handle));
#else
        return (dlclose(handle) == 0);
#endif
    }

    std::string getLastError() {
#ifdef WIN32
        std::ostringstream os;
        os << GetLastError();
        return os.str();
#else
        return dlerror();
#endif
    }
}// namespace

typedef void modelDescriptionTYPE(const char *);

int main(int argc, char **argv) {

    if (argc != 2) return 1;

    const std::string saveLocation = "../../modelDescription.xml";
    const std::string libName = argv[1];

    const auto handle = load_library((std::filesystem::current_path() / libName).string());

    if (!handle) {
        const auto err = "[fmu4cpp modelDescription generator] Unable to load dynamic library '" + libName + "'! " + getLastError();
        std::cerr << err << std::endl;
        return 1;
    }

    const auto f = load_function<modelDescriptionTYPE *>(handle, "write_description");
    f(saveLocation.c_str());

    free_library(handle);

    return 0;
}
