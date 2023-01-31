#!/bin/bash

echo "START BUILDING (HOST)"

echo "Cleaning previously built binary"
rm -f release-build/xahaud

BUILD_CORES=$(echo "scale=0 ; `nproc` / 3" | bc)

if [[ "$GITHUB_REPOSITORY" == "" ]]; then
  #Default
  BUILD_CORES=8
fi

echo "-- BUILD CORES:       $BUILD_CORES"
echo "-- GITHUB_REPOSITORY: $GITHUB_REPOSITORY"
echo "-- GITHUB_SHA:        $GITHUB_SHA"

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

STATIC_CONTAINER=$(docker ps -a | grep xahaud_cached_builder |wc -l)

if [[ "$STATIC_CONTAINER" -gt "0" ]]; then
  echo "Static container, execute in static container to have max. cache"
  docker exec -i xahaud_cached_builder /hbb_exe/activate-exec bash -x /io/build-core.sh "$GITHUB_REPOSITORY" "$GITHUB_SHA" "$BUILD_CORES"
else
  echo "No static container, build on temp container"
  docker run -i --user 0:$(id -g) --rm  -v `pwd`:/io --network host ghcr.io/foobarwidget/holy-build-box-x64 /hbb_exe/activate-exec bash -x /io/build-full.sh "$GITHUB_REPOSITORY" "$GITHUB_SHA" "$BUILD_CORES"
fi

echo "DONE BUILDING (HOST)"
