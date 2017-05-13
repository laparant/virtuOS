project(VirtuOS)
cmake_minimum_required(VERSION 2.8)

include_directories ("${PROJECT_SOURCE_DIR}/src")

# Variables used for the tests
set(NB_THREADS 10)
set(NB_YIELD 100)
set(FIBO 10)
set(THREAD_PRIORITY 8)

# test 01-main.c
add_executable(test_01_main test_01_main.c)
target_link_libraries (test_01_main thread)
add_test(tst01 test_01_main)

# test 02-switch.c
add_executable(test_02_switch test_02_switch.c)
target_link_libraries (test_02_switch thread)
add_test(tst02 test_02_switch)

# test 11-join.c
add_executable(test_11_join test_11_join.c)
target_link_libraries (test_11_join thread)
add_test(tst11 test_11_join)

# test 12-join-main.c
add_executable(test_12_join_main test_12_join_main.c)
target_link_libraries (test_12_join_main thread)
add_test(tst12 test_12_join_main)

# test 21-create-many.c
add_executable(test_21_create_many test_21_create_many.c)
target_link_libraries (test_21_create_many thread)
add_test(tst21 test_21_create_many ${NB_THREADS})

# test 22-create-many-recursive.c
add_executable(test_22_create_many_recursive test_22_create_many_recursive.c)
target_link_libraries (test_22_create_many_recursive thread)
add_test(tst22 test_22_create_many_recursive ${NB_THREADS})

# test 23-create-many-once.c
add_executable(test_23_create_many_once test_23_create_many_once.c)
target_link_libraries (test_23_create_many_once thread)
add_test(tst23 test_23_create_many_once ${NB_THREADS})

# test 31-switch-many.c
add_executable(test_31_switch_many test_31_switch_many.c)
target_link_libraries (test_31_switch_many thread)
add_test(tst31 test_31_switch_many ${NB_THREADS} ${NB_YIELD})

# test 32-switch-many-join.c
add_executable(test_32_switch_many_join test_32_switch_many_join.c)
target_link_libraries (test_32_switch_many_join thread)
add_test(tst32 test_32_switch_many_join ${NB_THREADS} ${NB_YIELD})

# test 51-fibonacci.c
add_executable(test_51_fibonacci test_51_fibonacci.c)
target_link_libraries (test_51_fibonacci thread)
add_test(tst51 test_51_fibonacci ${FIBO})

# test 61-mutex.c
add_executable(test_61_mutex test_61_mutex.c)
target_link_libraries (test_61_mutex thread)
add_test(tst61 test_61_mutex ${NB_THREADS})

# test_71_preemption
add_executable(test_71_preemption test_71_preemption.c)
target_link_libraries (test_71_preemption thread)
add_test(tst71 test_71_preemption)

# test_72_preemption
add_executable(test_72_preemption test_72_preemption.c)
target_link_libraries (test_72_preemption thread)
add_test(tst72 test_72_preemption)

# test_81_priority
add_executable(test_81_priority test_81_priority.c)
target_link_libraries (test_81_priority thread)
add_test(tst81 test_81_priority ${THREAD_PRIORITY})

# test_91_segfault
add_executable(test_91_segfault test_91_segfault.c)
target_link_libraries (test_91_segfault thread)
add_test(tst91 test_91_segfault)
