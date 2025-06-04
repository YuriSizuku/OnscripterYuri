# sh -c "export BUILD_TYPE=Debug && export MSYS2_HOME=/path/to/msys2 && ./local_msys2mingw32.sh"
BUILD_PATH=./../build_msys2mingw32
CMAKELISTS_PATH=./../
CORE_NUM=$(cat /proc/cpuinfo | grep -c ^processor)
TARGETS=$@

# config env
CC=$MSYS2_HOME/mingw32/bin/gcc
CXX=$MSYS2_HOME/mingw32/bin/g++
if [ -n "$(uname -a | grep Msys)" ]; then
    CC+=".exe"
    CXX+=".exe"
fi
PATH=$MSYS2_HOME/mingw32/bin/:$PATH
if [ -z "$BUILD_TYPE" ]; then BUILD_TYPE=MinSizeRel; fi
if [ -z "$TARGETS" ]; then TARGETS=all; fi

# config and build project
echo "BUILD_TYPE=$BUILD_TYPE, MSYS2_HOME=$MSYS2_HOME", CC=$CC
cmake -B $BUILD_PATH -S $CMAKELISTS_PATH \
    -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX \
    -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32
make -C $BUILD_PATH $TARGETS -j$CORE_NUM