find_package(etcd-cpp-api REQUIRED)
add_executable(s_test_etcd main.cpp)
target_link_libraries(s_test_etcd PRIVATE etcd-cpp-api cpprest)