# bash -c "BUILD_TYPE=Debug SKIP_PORTS=yes ./cross_linux64.sh"
PLATFORM=linux64
BUILD_PATH=./../build_${PLATFORM}
CMAKELISTS_PATH=$(pwd)/..
PORTBUILD_PATH=$CMAKELISTS_PATH/thirdparty/build/arch_$PLATFORM
CORE_NUM=$(cat /proc/cpuinfo | grep -c ^processor)
TARGETS=$@

# config env
if [ -z "$CC" ]; then
    export CC=x86_64-linux-gnu-gcc
    export CXX=x86_64-linux-gnu-g++
fi
if [ -z "$BUILD_TYPE" ]; then BUILD_TYPE=MinSizeRel; fi
if [ -z "$TARGETS" ]; then TARGETS=all; fi

# build ports
echo "BUILD_TYPE=$BUILD_TYPE"
echo "## PORTBUILD_PATH=$PORTBUILD_PATH"
if [ -z "$SKIP_PORTS" ]; then
    source _fetch.sh
    source _$PLATFORM.sh
    fetch_jpeg ; build_jpeg
    fetch_bz2 ; build_bz2
    fetch_lua ; build_lua
    fetch_sdl2 ; build_sdl2
    fetch_sdl2_image ; build_sdl2_image 
    fetch_sdl2_ttf ; build_sdl2_ttf
    fetch_sdl2_mixer ; build_sdl2_mixer
fi

# config project
cmake -B $BUILD_PATH -S $CMAKELISTS_PATH \
    -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX \
    -DSTATIC_PORT_ROOT=$PORTBUILD_PATH

# build project
make -C $BUILD_PATH $TARGETS -j$CORE_NUM