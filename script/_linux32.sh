# must use after _fetch.sh from local_linux32.sh

function build_lua()
{
    if ! [ -d "${LUA_SRC}_${PLATFORM}" ]; then cp -rp ${LUA_SRC} "${LUA_SRC}_${PLATFORM}"; fi
    LUA_SRC=${LUA_SRC}_${PLATFORM}
    echo "## LUA_SRC=$LUA_SRC"
    make -C $LUA_SRC all PLAT=linux CC="gcc -m32" -j$CORE_NUM
    make -C $LUA_SRC install INSTALL_TOP=$PORTBUILD_PATH
}

function build_jpeg()
{
    if ! [ -d "${JPEG_SRC}_${PLATFORM}" ]; then cp -rp ${JPEG_SRC} "${JPEG_SRC}_${PLATFORM}"; fi
    JPEG_SRC=${JPEG_SRC}_${PLATFORM}
    echo "## SDL2_SRC=$JPEG_SRC"
    export CFLAGS="-m32"
    pushd $JPEG_SRC
    ./configure --host=i386-linux-gnu \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM && make install 
    popd
}

function build_bz2()
{
    if ! [ -d "${BZ2_SRC}_${PLATFORM}" ]; then cp -rp ${BZ2_SRC} "${BZ2_SRC}_${PLATFORM}"; fi
    BZ2_SRC=${BZ2_SRC}_${PLATFORM}
    echo "## BZ2_SRC=$BZ2_SRC"
    make -C $BZ2_SRC all CC="gcc -m32" -j$CORE_NUM
    make -C $BZ2_SRC install PREFIX=$PORTBUILD_PATH
}

function build_sdl2() # after pulse
{
    if ! [ -d "${SDL2_SRC}_${PLATFORM}" ]; then cp -rp ${SDL2_SRC} "${SDL2_SRC}_${PLATFORM}"; fi
    SDL2_SRC=${SDL2_SRC}_${PLATFORM}
    echo "## SDL2_SRC=$SDL2_SRC"
    pushd $SDL2_SRC
    ./configure --host=i686-linux-gnu \
        "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32"\
        --disable-3dnow --disable-sse --disable-sse3 \
        --disable-video-opengles --disable-video-opengles1 \
        --disable-video-wayland --disable-video-offscreen \
        --enable-video-x11  --enable-x11-shared  \
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
    ./configure --host=i686-linux-gnu \
        "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32" \
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
    pushd $SDL2_TTF_SRC
    ./configure --host=i686-linux-gnu \
        "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32" \
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
    ./configure --host=i686-linux-gnu \
        "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32" \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM 
    make install 
    popd
}