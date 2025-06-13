# bash -c "BUILD_TYPE=Debug SKIP_PORTS=1 ./cross_llvmmingw32.sh"
PLATFORM=llvmmingw32
BUILD_PATH=./../build_${PLATFORM}
CMAKELISTS_PATH=$(pwd)/..
PORTBUILD_PATH=$CMAKELISTS_PATH/thirdparty/build/arch_$PLATFORM
if [ -z "$CORE_NUM" ]; then 
    CORE_NUM=$(cat /proc/cpuinfo | grep -c ^processor)
fi
TARGETS=$@

# config env
if [ -z "$CC" ]; then
    export CC=i686-w64-mingw32-clang
    export CXX=i686-w64-mingw32-clang++
    export RC=i686-w64-mingw32-windres
fi

if [ -z "$BUILD_TYPE" ]; then BUILD_TYPE=MinSizeRel; fi
if [ -z "$TARGETS" ]; then TARGETS=all; fi

echo "## PORTBUILD_PATH=$PORTBUILD_PATH"
if [ -z "$SKIP_PORTS" ]; then
    source _fetch.sh
    source _$PLATFORM.sh
    fetch_bz2 ; build_bz2
    fetch_lua ; build_lua
    fetch_sdl2 ; build_sdl2
    fetch_sdl2_image ; build_sdl2_image 
    fetch_sdl2_ttf ; build_sdl2_ttf
    fetch_sdl2_mixer ; build_sdl2_mixer
fi

# config project
echo "BUILD_TYPE=$BUILD_TYPE"
cmake -B $BUILD_PATH -S $CMAKELISTS_PATH \
    -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_C_COMPILER=$CC \
    -DCMAKE_CXX_COMPILER=$CXX \
    -DCMAKE_RC_COMPILER=$RC \
    -DCMAKE_SYSTEM_NAME="Windows" \
    -DSTATIC_PORT_ROOT=$PORTBUILD_PATH \
    -DEXTRA_SDL_LIB=ON

# build project
make -C $BUILD_PATH $TARGETS -j$CORE_NUM