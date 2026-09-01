cmake_minimum_required(VERSION 3.10)

project(ThreadPool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(thread_pool_demo
    main.cpp
    ThreadPool.cpp
    Result.cpp
    Semphore.cpp
    Task.cpp
    Any.cpp
)

target_link_libraries(thread_pool_demo PRIVATE Threads::Threads)