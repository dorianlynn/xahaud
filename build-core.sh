#!/bin/bash

echo "START INSIDE CONTAINER - CORE"

echo "-- BUILD CORES:       $3"
echo "-- GITHUB_REPOSITORY: $1"
echo "-- GITHUB_SHA:        $2"

umask 0000;

cd /io/ &&
echo "-- Build Rippled --" &&
pwd &&
cp Builds/CMake/deps/Rocksdb.cmake Builds/CMake/deps/Rocksdb.cmake.old &&
perl -i -pe "s/^(\\s*)-DBUILD_SHARED_LIBS=OFF/\\1-DBUILD_SHARED_LIBS=OFF\\n\\1-DROCKSDB_BUILD_SHARED=OFF/g" Builds/CMake/deps/Rocksdb.cmake &&
mv Builds/CMake/deps/WasmEdge.cmake Builds/CMake/deps/WasmEdge.old &&
echo "find_package(LLVM REQUIRED CONFIG)
message(STATUS \"Found LLVM ${LLVM_PACKAGE_VERSION}\")
message(STATUS \"Using LLVMConfig.cmake in: \${LLVM_DIR}\")
add_library (wasmedge STATIC IMPORTED GLOBAL)
set_target_properties(wasmedge PROPERTIES IMPORTED_LOCATION \${WasmEdge_LIB})
target_link_libraries (ripple_libs INTERFACE wasmedge)
add_library (NIH::WasmEdge ALIAS wasmedge)
message(\"WasmEdge DONE\")
" > Builds/CMake/deps/WasmEdge.cmake &&
cd release-build &&
cmake .. -DBoost_NO_BOOST_CMAKE=ON -DLLVM_DIR=/usr/lib64/llvm13/lib/cmake/llvm/ -DLLVM_LIBRARY_DIR=/usr/lib64/llvm13/lib/ -DWasmEdge_LIB=/usr/local/lib64/libwasmedge.a &&
make -j$3 VERBOSE=1 &&
strip -s rippled &&
mv rippled xahaud &&
echo "Build host: `hostname`" > release.info &&
echo "Build date: `date`" >> release.info &&
echo "Build md5: `md5sum xahaud`" >> release.info &&
echo "Git remotes:" >> release.info && 
git remote -v >> release.info 
echo "Git status:" >> release.info &&
git status -v >> release.info &&
echo "Git log [last 20]:" >> release.info &&
git log -n 20 >> release.info;
cd ..;
mv src/ripple/net/impl/RegisterSSLCerts.cpp.old src/ripple/net/impl/RegisterSSLCerts.cpp;
mv Builds/CMake/deps/Rocksdb.cmake.old Builds/CMake/deps/Rocksdb.cmake;
mv Builds/CMake/deps/WasmEdge.old Builds/CMake/deps/WasmEdge.cmake;

echo "END INSIDE CONTAINER - CORE"
