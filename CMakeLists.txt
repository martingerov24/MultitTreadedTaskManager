project(TaskSystemExample)

SET(PLUGIN_INSTALL_PATH "${CMAKE_BINARY_DIR}/ts_executors" CACHE STRING "Path to collect dynamic libs and application")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(TaskSystem)
add_subdirectory(PrinterExecutor)
add_subdirectory(RaytracerExecutor)