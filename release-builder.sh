#!/bin/bash

echo "START BUILDING (HOST)"

BUILD_CORES=8

if [[ "$GITHUB_REPOSITORY" -eq "" ]]; then
  BUILD_CORES=$(echo "scale=0 ; `nproc` / 3" | bc)
fi

echo "-- GITHUB_REPOSITORY: $GITHUB_REPOSITORY"
echo "-- GITHUB_SHA:        $GITHUB_SHA"
echo "-- BUILD CORES:       $BUILD_CORES"

docker run -i --user 0:$(id -g) --rm  -v `pwd`:/io --network host ghcr.io/foobarwidget/holy-build-box-x64 /hbb_exe/activate-exec bash -x /io/build-full.sh "$GITHUB_REPOSITORY" "$GITHUB_SHA" "$BUILD_CORES"

exit 0

which docker 2> /dev/null 2> /dev/null
if [ "$?" -eq "1" ]
then
  echo 'Docker not found. Install it first.'
  exit 1
fi

stat .git 2> /dev/null 2> /dev/null
if [ "$?" -eq "1" ]
then
  echo 'Run this inside the source directory. (.git dir not found).'
  exit 1
fi
docker run -i --user 0:$(id -g) --rm  -v `pwd`:/io --network host ghcr.io/foobarwidget/holy-build-box-x64 /hbb_exe/activate-exec bash -x /io/build-full.sh "$GITHUB_REPOSITORY" "$GITHUB_SHA" "$BUILD_CORES"

echo "DONE BUILDING (HOST)"
