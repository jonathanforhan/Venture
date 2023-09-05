file(GLOB_RECURSE SOURCE "*.c")
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE .)
