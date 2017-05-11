project(VirtuOS)
cmake_minimum_required(VERSION 2.8)

include_directories ("${PROJECT_SOURCE_DIR}/src")

# test 01-main.c
add_executable(test01 test_01_main.c)
target_link_libraries (test01 thread)

# test 02-switch.c
add_executable(test02 test_02_switch.c)
target_link_libraries (test02 thread)

# test 11-join.c
add_executable(test11 test_11_join.c)
target_link_libraries (test11 thread)

# test 12-join-main.c
add_executable(test12 test_12_join_main.c)
target_link_libraries (test12 thread)

# test 21-create-many.c
add_executable(test21 test_21_create_many.c)
target_link_libraries (test21 thread)

# test 22-create-many-recursive.c
add_executable(test22 test_22_create_many_recursive.c)
target_link_libraries (test22 thread)

# test 23-create-many-once.c
add_executable(test23 test_23_create_many_once.c)
target_link_libraries (test23 thread)

# test 31-switch-many.c
add_executable(test31 test_31_switch_many.c)
target_link_libraries (test31 thread)

# test 32-switch-many-join.c
add_executable(test32 test_32_switch_many_join.c)
target_link_libraries (test32 thread)

# test 51-fibonacci.c
add_executable(test51 test_51_fibonacci.c)
target_link_libraries (test51 thread)

# test_71_preemption
add_executable(test_71_preemption test_71_preemption.c)
target_link_libraries (test_71_preemption thread)

# test_72_preemption
add_executable(test_72_preemption test_72_preemption.c)
target_link_libraries (test_72_preemption thread)

# TO ADD when thread_mutex_unlock, thread_mutex_init and thread_mutex_destroy will be done
# test 61-mutex.c
#add_executable(test61 test_61_mutex.c)
#target_link_libraries (test61 thread)
