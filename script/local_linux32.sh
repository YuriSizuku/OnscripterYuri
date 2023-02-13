# bash -c "export BUILD_TYPE=Debug && export USE_STATIC_PORTS=yes && export SKIP_PORTS=yes && ./local_linux32.sh"
PLATFORM=linux32
BUILD_PATH=./../build_${PLATFORM}
CMAKELISTS_PATH=$(pwd)/..
PORTBUILD_PATH=$CMAKELISTS_PATH/thirdparty/build/arch_$PLATFORM
CORE_NUM=$(cat /proc/cpuinfo | grep -c ^processor)
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
if [ -n "USE_STATIC_PORTS" ]; then
    echo "## PORTBUILD_PATH=$PORTBUILD_PATH"
    if [ -z "$SKIP_PORTS" ]; then
        source _fetch.sh
        source _$PLATFORM.sh
        fetch_jpeg && build_jpeg
        fetch_bz2 && build_bz2
        fetch_lua && build_lua
        fetch_sdl2 && build_sdl2
        fetch_sdl2_image && build_sdl2_image 
        fetch_sdl2_ttf && build_sdl2_ttf
        fetch_sdl2_mixer && build_sdl2_mixer
    fi
    cmake -B $BUILD_PATH -S $CMAKELISTS_PATH \
        -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX \
        -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32 \
        -DSTATIC_PORT_ROOT=$PORTBUILD_PATH
else # build without ports 
    cmake -B $BUILD_PATH -S $CMAKELISTS_PATH \
        -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX \
        -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32
fi

make -C $BUILD_PATH $TARGETS -j$CORE_NUM