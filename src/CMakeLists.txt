file(GLOB_RECURSE SOURCE "*.cpp")
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE .)