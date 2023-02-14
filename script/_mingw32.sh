# must use after _fetch.sh from cross_mingw32.sh

function build_lua()
{
    if ! [ -d "${LUA_SRC}_${PLATFORM}" ]; then cp -rp ${LUA_SRC} "${LUA_SRC}_${PLATFORM}"; fi
    LUA_SRC=${LUA_SRC}_${PLATFORM}
    echo "## LUA_SRC=$LUA_SRC"
    make -C $LUA_SRC all PLAT=mingw -j$CORE_NUM \
        CC=i686-w64-mingw32-gcc AR="i686-w64-mingw32-ar rcu"
    cp $LUA_SRC/src/lua.exe  $LUA_SRC/src/lua
    cp $LUA_SRC/src/luac.exe  $LUA_SRC/src/luac
    make -C $LUA_SRC install INSTALL_TOP=$PORTBUILD_PATH
}

function build_jpeg()
{
    if ! [ -d "${JPEG_SRC}_${PLATFORM}" ]; then cp -rp ${JPEG_SRC} "${JPEG_SRC}_${PLATFORM}"; fi
    JPEG_SRC=${JPEG_SRC}_${PLATFORM}
    echo "## SDL2_SRC=$JPEG_SRC"
    pushd $JPEG_SRC
    ./configure --host=i686-w64-mingw32 \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM && make install 
    popd
}

function build_bz2()
{
    if ! [ -d "${BZ2_SRC}_${PLATFORM}" ]; then cp -rp ${BZ2_SRC} "${BZ2_SRC}_${PLATFORM}"; fi
    BZ2_SRC=${BZ2_SRC}_${PLATFORM}
    echo "## BZ2_SRC=$BZ2_SRC CC=$CC"
    make -C $BZ2_SRC -j$CORE_NUM # this has some problem by mingw
    make -C $BZ2_SRC install PREFIX=$PORTBUILD_PATH

    pushd $CMAKELISTS_PATH/thirdparty/port/
    curl -fsSL https://mirror.msys2.org/mingw/mingw32/mingw-w64-i686-bzip2-1.0.8-2-any.pkg.tar.zst -O
    tar xf mingw-w64-i686-bzip2-1.0.8-2-any.pkg.tar.zst mingw32/lib/libbz2.a
    cp mingw32/lib/libbz2.a $PORTBUILD_PATH/lib/
    rm -rf mingw32
    popd
}


function build_sdl2() 
{
    if ! [ -d "${SDL2_SRC}_${PLATFORM}" ]; then cp -rp ${SDL2_SRC} "${SDL2_SRC}_${PLATFORM}"; fi
    SDL2_SRC=${SDL2_SRC}_${PLATFORM}
    echo "## SDL2_SRC=$SDL2_SRC"
    export CFLAGS="-Os"
    export CXXFLAGS="-Os"
    pushd $SDL2_SRC
    ./configure --host=i686-w64-mingw32 \
        --disable-3dnow --disable-sse --disable-sse3 \
        --disable-video-vulkan --disable-video-offscreen \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM && make install 
    popd
}

function build_sdl2_image() # after build_sdl2
{
    if ! [ -d "${SDL2_IMAGE_SRC}_${PLATFORM}" ]; then cp -rp ${SDL2_IMAGE_SRC} "${SDL2_IMAGE_SRC}_${PLATFORM}"; fi
    SDL2_IMAGE_SRC=${SDL2_IMAGE_SRC}_${PLATFORM}
    echo "## SDL2_IMAGE_SRC=$SDL2_IMAGE_SRC"

    export PKG_CONFIG_PATH=${PORTBUILD_PATH}/lib/pkgconfig # this is inportant for find SDL path    
    pushd $SDL2_IMAGE_SRC
    ./configure --host=i686-w64-mingw32 \
        --enable-stb_image \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM &&  make install  # stb_image.h already included
    popd
}

function build_sdl2_ttf() # after build_sdl2
{
    if ! [ -d "${SDL2_TTF_SRC}_${PLATFORM}" ]; then cp -rp ${SDL2_TTF_SRC} "${SDL2_TTF_SRC}_${PLATFORM}"; fi
    SDL2_TTF_SRC=${SDL2_TTF_SRC}_${PLATFORM}
    echo "## SDL2_TTF_SRC=$SDL2_TTF_SRC"

    export PKG_CONFIG_PATH=${PORTBUILD_PATH}/lib/pkgconfig # this is inportant for find SDL path    
    pushd $SDL2_TTF_SRC # harfbuzz makes very large
    ./configure --host=i686-w64-mingw32 \
        --disable-harfbuzz \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM 
    make install 
    popd
}

function build_sdl2_mixer() # after build_sdl2
{
    if ! [ -d "${SDL2_MIXER_SRC}_${PLATFORM}" ]; then cp -rp ${SDL2_MIXER_SRC} "${SDL2_MIXER_SRC}_${PLATFORM}"; fi
    SDL2_MIXER_SRC=${SDL2_MIXER_SRC}_${PLATFORM}
    echo "## SDL2_MIXER_SRC=$SDL2_MIXER_SRC"

    export PKG_CONFIG_PATH=${PORTBUILD_PATH}/lib/pkgconfig # this is inportant for find SDL path    
    pushd $SDL2_MIXER_SRC
    ./configure --host=i686-w64-mingw32 \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM 
    make install 
    popd
}