#!/bin/bash

echo "START INSIDE CONTAINER - CORE"

echo "-- BUILD CORES:       $3"
echo "-- GITHUB_REPOSITORY: $1"
echo "-- GITHUB_SHA:        $2"
echo "-- GITHUB_RUN_NUMBER: $4"

umask 0000;

export PATH=`echo $PATH | sed -E "s/devtoolset-9/devtoolset-10/g"` &&
cd /io/ &&
git checkout src/ripple/protocol/impl/BuildInfo.cpp &&
sed -i s/\"0.0.0\"/\"$(date +%s)-$(git rev-parse --abbrev-ref HEAD)-$4\"/g src/ripple/protocol/impl/BuildInfo.cpp &&
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
