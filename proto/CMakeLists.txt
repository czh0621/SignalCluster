find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

set(GEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/gen")

# -----------------------------
# 构建阶段调用生成脚本
# -----------------------------

# -----------------------------
# 收集生成的 pb 文件
# -----------------------------
file(GLOB_RECURSE GENERATED_SRCS
        "${GEN_DIR}/*.pb.cc"
        "${GEN_DIR}/*.grpc.pb.cc"
)
file(GLOB_RECURSE GENERATED_HDRS
        "${GEN_DIR}/*.pb.h"
        "${GEN_DIR}/*.grpc.pb.h"
)

# -----------------------------
# 静态库
# -----------------------------
file(GLOB GEN_SUBDIRS RELATIVE ${GEN_DIR} ${GEN_DIR}/*)

set(INCLUDE_DIRS ${GEN_DIR})
foreach (subdir ${GEN_SUBDIRS})
    if (IS_DIRECTORY "${GEN_DIR}/${subdir}")
        list(APPEND INCLUDE_DIRS "${GEN_DIR}/${subdir}")
    endif ()
endforeach ()
add_library(proto_lib STATIC ${GENERATED_SRCS} ${GENERATED_HDRS})
target_include_directories(proto_lib PUBLIC ${INCLUDE_DIRS})
target_link_libraries(proto_lib PUBLIC protobuf::libprotobuf gRPC::grpc++)