# bash -c "export BUILD_TYPE=Debug && export USE_STATIC_PORTS=yes && export SKIP_PORTS=yes && ./local_linux64.sh"
PLATFORM=darwin
BUILD_PATH=./../build_${PLATFORM}
CMAKELISTS_PATH=$(pwd)/..
CORE_NUM=$(nproc)
TARGETS=$@

# config env
CC=gcc
CXX=g++
if [ -z "$BUILD_TYPE" ]; then BUILD_TYPE=MinSizeRel; fi
if [ -z "$TARGETS" ]; then TARGETS=all; fi

# config and build project
# USE_STATIC_PORTS=yes
# SKIP_PORTS=yes
echo "BUILD_TYPE=$BUILD_TYPE"
  cmake -B $BUILD_PATH -S $CMAKELISTS_PATH \
      -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE \

make -C $BUILD_PATH $TARGETS -j$CORE_NUM