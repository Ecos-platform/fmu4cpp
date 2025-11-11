
Include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 12.1.0
)
if (DEFINED BUILD_SHARED_LIBS)
    set(_fmt_prev_BUILD_SHARED_LIBS "${BUILD_SHARED_LIBS}")
    set(_fmt_prev_was_cached TRUE)
else()
    set(_fmt_prev_was_cached FALSE)
endif()

set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)
FetchContent_MakeAvailable(fmt)

if (_fmt_prev_was_cached)
    set(BUILD_SHARED_LIBS "${_fmt_prev_BUILD_SHARED_LIBS}" CACHE BOOL "Build shared libraries" FORCE)
else()
    unset(BUILD_SHARED_LIBS CACHE)
endif()

generateFMU(external_lib
        SOURCES
        "external_lib.cpp"
        LINK_TARGETS
        fmt::fmt
        FMI_VERSIONS
        "fmi2"
        "fmi3"
)
