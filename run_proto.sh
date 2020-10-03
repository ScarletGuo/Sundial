protoc -I ./grpc_system --grpc_out=./grpc_system --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ./grpc_system/sundial_grpc.proto
protoc -I ./grpc_system --cpp_out=./grpc_system ./grpc_system/sundial_grpc.proto
