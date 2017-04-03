project(VirtuOS LANGUAGES C)
cmake_minimum_required(VERSION 2.8)

add_subdirectory("src")
add_subdirectory("tst")
add_subdirectory("examples")

enable_testing()
