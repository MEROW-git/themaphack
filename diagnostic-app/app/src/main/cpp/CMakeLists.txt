cmake_minimum_required(VERSION 3.22)
project(themaphack_diagnostics LANGUAGES CXX)

add_library(themaphack_diagnostics SHARED diagnostics.cpp)
target_compile_features(themaphack_diagnostics PRIVATE cxx_std_17)
target_compile_options(themaphack_diagnostics PRIVATE -Wall -Wextra)
