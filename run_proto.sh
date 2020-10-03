dirname="proto"
protoc -I ./${dirname} --grpc_out=./${dirname} --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ./${dirname}/sundial_grpc.proto
protoc -I ./${dirname} --cpp_out=./${dirname} ./${dirname}/sundial_grpc.proto
cd ${dirname}
mv sundial_grpc.grpc.pb.cc sundial_grpc.grpc.pb.cpp
mv  sundial_grpc.pb.cc  sundial_grpc.pb.cpp
