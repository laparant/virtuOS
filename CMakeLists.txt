project(VirtuOS LANGUAGES C)
cmake_minimum_required(VERSION 3.1)

add_subdirectory("src")
add_subdirectory("tst")
add_subdirectory("examples")

enable_testing()