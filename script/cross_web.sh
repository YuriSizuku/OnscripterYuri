BUILD_PATH=./../build_web
CMAKELISTS_PATH=./../

# prepare libs
if ! [ -d ./../externlib ]; then mkdir ./../externlib; fi
if ! [ -d ./../externlib/stb ]; then git clone https://github.com/nothings/stb.git ./../externlib/stb; fi
if ! [ -d ./../externlib/glm ]; then git clone https://github.com/g-truc/glm.git ./../externlib/glm; fi

# config env
if [ -z "$EMCSDK" ]; then EMCSDK=/d/Software/env/sdk/emsdk; fi
if [ -n "$(uname -a | grep MSYS)" ]; then # fix python problem in windows
    if [ -z "$MSYS2SDK" ]; then MSYS2SDK=/d/Software/env/msys2/; fi; 
    PATH=$MSYS2SDK/mingw32/bin/:$PATH
fi
source "$EMCSDK/emsdk_env.sh"
if [ -z "$BUILD_TYPE" ]; then BUILD_TYPE=MinSizeRel; fi

# config and build project
rm -rf $BUILD_PATH/*
emcmake cmake -G "Unix Makefiles" \
    -S $CMAKELISTS_PATH -B $BUILD_PATH \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE
make -C $BUILD_PATH circle_danmaku gl_phong_demo