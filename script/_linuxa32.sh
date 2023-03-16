# must use after _fetch.sh from cross_linuxa64.sh

function build_lua()
{
    echo "## LUA_SRC=$LUA_SRC"

    make -C $LUA_SRC clean
    make -C $LUA_SRC all PLAT=linux CC=arm-linux-gnueabihf-gcc AR="arm-linux-gnueabihf-ar rcu" -j$CORE_NUM
    make -C $LUA_SRC install INSTALL_TOP=$PORTBUILD_PATH
}

function build_jpeg()
{
    if ! [ -d "${JPEG_SRC}/build_${PLATFORM}" ]; then mkdir -p "${JPEG_SRC}/build_${PLATFORM}"; fi
    echo "## JPEG_SRC=$JPEG_SRC"

    pushd "${JPEG_SRC}/build_${PLATFORM}"
    ../configure --host=arm-linux-gnueabihf \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM && make install 
    popd
}

function build_bz2()
{
    echo "## BZ2_SRC=$BZ2_SRC"
    
    make -C $BZ2_SRC clean
    make -C $BZ2_SRC all CC=arm-linux-gnueabihf-gcc AR=arm-linux-gnueabihf-ar -j$CORE_NUM
    make -C $BZ2_SRC install PREFIX=$PORTBUILD_PATH
}

function build_sdl2()
{
    if ! [ -d "${SDL2_SRC}/build_${PLATFORM}" ]; then mkdir -p "${SDL2_SRC}/build_${PLATFORM}"; fi
    echo "## SDL2_SRC=$SDL2_SRC"

    export CFLAGS="-Os"
    export CXXFLAGS="-Os"
    SDL2_SYSROOT=/
    if [ -n "$SYSROOT" ]; then SDL2_SYSROOT=$SYSROOT; fi
    echo "## SDL2_SYSROOT $SDL2_SYSROOT"
    pushd "${SDL2_SRC}/build_${PLATFORM}"
    ../configure --host=arm-linux-gnueabihf \
        --disable-pulseaudio \
        --enable-video-x11  --enable-x11-shared  --enable-video-x11-xcursor --enable-video-x11-xinput --enable-video-x11-xrandr \
        --disable-video-wayland \
        --enable-arm-simd --enable-arm-neon \
        --prefix=$PORTBUILD_PATH --with-sysroot=$SDL2_SYSROOT
    make -j$CORE_NUM && make install 
    popd
}

# after build_sdl2
function build_sdl2_image()
{
    if ! [ -d "${SDL2_IMAGE_SRC}/build_${PLATFORM}" ]; then mkdir -p "${SDL2_IMAGE_SRC}/build_${PLATFORM}"; fi
    echo "## SDL2_IMAGE_SRC=$SDL2_IMAGE_SRC"

    export PKG_CONFIG_PATH=${PORTBUILD_PATH}/lib/pkgconfig # this is important for find SDL path    
    pushd "${SDL2_IMAGE_SRC}/build_${PLATFORM}"
    ../configure --host=arm-linux-gnueabihf \
        --enable-stb_image \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM &&  make install
    popd
}

# after build_sdl2
function build_sdl2_ttf() 
{
    if ! [ -d "${SDL2_TTF_SRC}/build_${PLATFORM}" ]; then mkdir -p "${SDL2_TTF_SRC}/build_${PLATFORM}"; fi
    echo "## SDL2_TTF_SRC=$SDL2_TTF_SRC"

    export PKG_CONFIG_PATH=${PORTBUILD_PATH}/lib/pkgconfig # this is important for find SDL path    
    # harfbuzz makes very large 
    pushd "${SDL2_TTF_SRC}//build_${PLATFORM}" 
    ../configure --host=arm-linux-gnueabihf \
        --disable-harfbuzz \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM && make install 
    popd
}

# after build_sdl2
function build_sdl2_mixer() 
{
    if ! [ -d "${SDL2_MIXER_SRC}/build_${PLATFORM}" ]; then mkdir -p "${SDL2_MIXER_SRC}/build_${PLATFORM}"; fi
    echo "## SDL2_MIXER_SRC=$SDL2_MIXER_SRC"

    export PKG_CONFIG_PATH=${PORTBUILD_PATH}/lib/pkgconfig # this is important for find SDL path    
    pushd "${SDL2_MIXER_SRC}/build_${PLATFORM}"
    ../configure --host=arm-linux-gnueabihf \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM && make install 
    popd
}