project(VirtuOS)
cmake_minimum_required(VERSION 2.8)

include_directories ("${PROJECT_SOURCE_DIR}/src")

# Variables used for the tests
set(NB_THREADS 10)
set(NB_YIELD 100)
set(FIBO 10)

# test 01-main.c
add_executable(test01 test_01_main.c)
target_link_libraries (test01 thread)
add_test(tst01 test01)

# test 02-switch.c
add_executable(test02 test_02_switch.c)
target_link_libraries (test02 thread)
add_test(tst02 test02)

# test 11-join.c
add_executable(test11 test_11_join.c)
target_link_libraries (test11 thread)
add_test(tst11 test11)

# test 12-join-main.c
add_executable(test12 test_12_join_main.c)
target_link_libraries (test12 thread)
add_test(tst12 test12)

# test 21-create-many.c
add_executable(test21 test_21_create_many.c)
target_link_libraries (test21 thread)
add_test(tst21 test21 ${NB_THREADS})

# test 22-create-many-recursive.c
add_executable(test22 test_22_create_many_recursive.c)
target_link_libraries (test22 thread)
add_test(tst22 test22 ${NB_THREADS})

# test 23-create-many-once.c
add_executable(test23 test_23_create_many_once.c)
target_link_libraries (test23 thread)
add_test(tst23 test23 ${NB_THREADS})

# test 31-switch-many.c
add_executable(test31 test_31_switch_many.c)
target_link_libraries (test31 thread)
add_test(tst31 test31 ${NB_THREADS} ${NB_YIELD})

# test 32-switch-many-join.c
add_executable(test32 test_32_switch_many_join.c)
target_link_libraries (test32 thread)
add_test(tst32 test32 ${NB_THREADS} ${NB_YIELD})

# test 51-fibonacci.c
add_executable(test51 test_51_fibonacci.c)
target_link_libraries (test51 thread)
add_test(tst51 test51 ${FIBO})

# test 61-mutex.c
add_executable(test61 test_61_mutex.c)
target_link_libraries (test61 thread)
add_test(tst61 test61 ${NB_THREADS})
