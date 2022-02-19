cmake_minimum_required (VERSION 3.14)
project (GCPT)

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    SET( PTR_ARCH 64 )
else( CMAKE_SIZEOF_VOID_P EQUAL 8 ) 
    SET( PTR_ARCH 32 )
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

seT(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<IF:$<CONFIG:Debug>,debug,rel>${PTR_ARCH})

set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/thirdparty/installed)
find_package(freetype CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>)

add_subdirectory (src/shared)
add_subdirectory (src/server)
add_subdirectory (src/client)
