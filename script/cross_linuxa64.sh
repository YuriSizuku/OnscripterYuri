# sh -c "export BUILD_TYPE=Debug && export SKIP_PORTS=1 && ./local_linux64.sh"
PLATFORM=linuxa64
BUILD_PATH=./../build_${PLATFORM}
CMAKELISTS_PATH=$(pwd)/..
PORTBUILD_PATH=$CMAKELISTS_PATH/thirdparty/build/arch_$PLATFORM
CORE_NUM=$(cat /proc/cpuinfo | grep -c ^processor)
TARGETS=$@

# config env
CC=aarch64-linux-gnu-gcc
CXX=aarch64-linux-gnu-g++
if [ -z "$BUILD_TYPE" ]; then BUILD_TYPE=MinSizeRel; fi
if [ -z "$TARGETS" ]; then TARGETS=all; fi

# SKIP_PORTS="yes"
echo "## PORTBUILD_PATH=$PORTBUILD_PATH"
if [ -z "$SKIP_PORTS" ]; then
    source _fetch.sh
    source _$PLATFORM.sh
    # https://swarminglogic.com/article/2014_11_crosscompile2
    fetch_jpeg && build_jpeg
    fetch_bz2 && build_bz2
    fetch_lua && build_lua
    get_pulse
    fetch_sdl2 && build_sdl2
    fetch_sdl2_image && build_sdl2_image 
    fetch_sdl2_ttf && build_sdl2_ttf
    fetch_sdl2_mixer && build_sdl2_mixer
fi

# config and build project
echo "BUILD_TYPE=$BUILD_TYPE"
PKG_CONFIG_PATH=${PORTBUILD_PATH}/lib/pkgconfig
cmake -B $BUILD_PATH -S $CMAKELISTS_PATH \
    -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX \
    -DCMAKE_SYSROOT=$PORTBUILD_PATH -DLINK_STATIC=1
make -C $BUILD_PATH $TARGETS -j$CORE_NUM