# sh -c "export BUILD_TYPE=Debug && export EMSDK=/d/Software/env/sdk/emsdk && export MSYS2SDK=/d/Software/env/msys2 && ./cross_web.sh"
BUILD_PATH=$(pwd)/../build_web
CMAKELISTS_PATH=$(pwd)/..
PORTBUILD_PATH=$CMAKELISTS_PATH/thirdparty/build/arch_wasm
CORE_NUM=$(cat /proc/cpuinfo | grep -c ^processor)
TARGETS=$@

# config env
source ./_fetch.sh
if [ -z "$EMSDK" ]; then EMSDK=/d/Software/env/sdk/emsdk; fi
if [ -n "$(uname -a | grep Msys)" ]; then # fix python problem in windows
    if [ -z "$MSYS2SDK" ]; then MSYS2SDK=/d/Software/env/msys2; fi
    PATH=$MSYS2SDK/mingw32/bin/:$PATH
fi
source "$EMSDK/emsdk_env.sh"
if [ -z "$BUILD_TYPE" ]; then BUILD_TYPE=MinSizeRel; fi
if [ -z "$TARGETS" ]; then TARGETS=all; fi

# build ports
function build_lua()
{
    if ! [ -d "${LUA_SRC}_wasm" ]; then cp -rp ${LUA_SRC} ${LUA_SRC}_wasm; fi
    LUA_SRC=${LUA_SRC}_wasm
    echo "## LUA_SRC=$LUA_SRC"
    if ! [ -d "$PORTBUILD_PATH/lua" ]; then mkdir -p "$PORTBUILD_PATH/lua"; fi
    make -C $LUA_SRC all PLAT=linux CC=emcc AR="emar rcu" -j$CORE_NUM
    make -C $LUA_SRC install INSTALL_TOP=$PORTBUILD_PATH -j$CORE_NUM
}

fetch_lua && build_lua
embuilder build sdl2 sdl2_ttf sdl2_image sdl2_mixer bzip2 ogg vorbis mpg123
emcc $CMAKELISTS_PATH/src/onsyuri_web/dummy.c \
    -o $BUILD_PATH/dummy.js \
    -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS=bmp,png,jpg 

# config and build project
# rm -rf $BUILD_PATH/*
if [ -n "$GAME_PATH" ]; then cp -rf $GAME_PATH/* $BUILD_PATH; fi
emcmake cmake -G "Unix Makefiles" \
    -S $CMAKELISTS_PATH -B $BUILD_PATH \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE
make -C $BUILD_PATH $TARGETS -j$CORE_NUM