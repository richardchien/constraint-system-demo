cmake_minimum_required(VERSION 3.14)
project(constraint-system-demo)

add_executable(main main.cpp)
target_compile_features(main PRIVATE cxx_std_20)
