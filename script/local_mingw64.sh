# sh -c "export BUILD_TYPE=Debug && export MSYS2SDK=/d/software/env/msys2 && ./local_mingw32.sh"
BUILD_PATH=./../build_mingw64
CMAKELISTS_PATH=./../
CORE_NUM=$(cat /proc/cpuinfo | grep -c ^processor)
TARGETS=$@

# config env
if [ -z "$MSYS2SDK" ]; then MSYS2SDK=/d/Software/env/msys2; fi;
CC=$MSYS2SDK/mingw64/bin/gcc
CXX=$MSYS2SDK/mingw64/bin/g++
if [ -n "$(uname -a | grep Msys)" ]; then
    CC+=".exe"
    CXX+=".exe"
fi
PATH=$MSYS2SDK/mingw64/bin/:$PATH
if [ -z "$BUILD_TYPE" ]; then BUILD_TYPE=MinSizeRel; fi
if [ -z "$TARGETS" ]; then TARGETS=all; fi

# config and build project
echo "BUILD_TYPE=$BUILD_TYPE, MSYS2SDK=$MSYS2SDK", CC=$CC
cmake -B $BUILD_PATH -S $CMAKELISTS_PATH \
    -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX \
    -DCMAKE_C_FLAGS=-m64 -DCMAKE_CXX_FLAGS=-m64
make -C $BUILD_PATH $TARGETS -j$CORE_NUM