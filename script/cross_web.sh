# sh -c "export BUILD_TYPE=Debug && export EMCSDK=/d/Software/env/sdk/emsdk && export MSYS2SDK=/d/Software/env/msys2 && ./cross_web.sh"
BUILD_PATH=./../build_web
CMAKELISTS_PATH=./../
TARGETS=$@

# prepare libs
if ! [ -d ./../thirdparty/src ]; then mkdir -p ./../thirdparty/src; fi
if ! [ -d ./../thirdparty/src/lua ]; then 
    git clone https://github.com/lua/lua.git ./../thirdparty/src/lua
fi

# config env
if [ -z "$EMCSDK" ]; then EMCSDK=/d/Software/env/sdk/emsdk; fi
if [ -n "$(uname -a | grep Msys)" ]; then # fix python problem in windows
    if [ -z "$MSYS2SDK" ]; then MSYS2SDK=/d/Software/env/msys2; fi; 
    PATH=$MSYS2SDK/mingw32/bin/:$PATH
fi
source "$EMCSDK/emsdk_env.sh"
if [ -z "$BUILD_TYPE" ]; then BUILD_TYPE=MinSizeRel; fi
if [ -z "$TARGETS" ]; then TARGETS=all; fi

# config and build project
# rm -rf $BUILD_PATH/*
embuilder build sdl2 sdl2_ttf sdl2_image sdl2_mixer bzip2 ogg vorbis mpg123
emcc src/onsjh_web/dummy.c \ # for get libjpg library
    -o $BUILD_PATH/dummy.js \
    -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS=bmp,png,jpg  

emcmake cmake -G "Unix Makefiles" \
    -S $CMAKELISTS_PATH -B $BUILD_PATH \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE
make -C $BUILD_PATH $TARGETS -j $(cat /proc/cpuinfo | grep -c ^processor)