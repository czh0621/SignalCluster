#!/usr/bin/env bash
set -e

PROTO_ROOT="$(pwd)"
GEN_DIR="${PROTO_ROOT}/gen"

mkdir -p "$GEN_DIR"

# 遍历根目录下所有子目录
find "$PROTO_ROOT" -mindepth 1 -maxdepth 1 -type d | while read -r SUB_DIR; do
    # 遍历子目录下所有 proto 文件
    find "$SUB_DIR" -name "*.proto" | while read -r PROTO_FILE; do
        # 相对路径，相对于子目录根
        REL_PATH="${PROTO_FILE#$SUB_DIR/}"

        # 生成对应的 gen 目录，保持原目录结构
        GEN_SUBDIR="$GEN_DIR/${SUB_DIR#$PROTO_ROOT/}/$(dirname "$REL_PATH")"
        mkdir -p "$GEN_SUBDIR"

        # 进入 proto 所在子目录编译，-I. 确保 import 可以找到
        (
            cd "$SUB_DIR" || exit
            protoc \
                -I. \
                --cpp_out="$GEN_DIR/${SUB_DIR#$PROTO_ROOT/}" \
                --grpc_out="$GEN_DIR/${SUB_DIR#$PROTO_ROOT/}" \
                --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) \
                "$REL_PATH"
        )

        echo "Generated: $SUB_DIR/$REL_PATH -> $GEN_SUBDIR"
    done
done

echo "All proto files generated successfully!"
