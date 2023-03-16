# must use after _fetch.sh from local_linux32.sh

function build_lua()
{
    echo "## LUA_SRC=$LUA_SRC"

    make -C $LUA_SRC clean
    make -C $LUA_SRC all PLAT=linux CC="gcc -m32" -j$CORE_NUM
    make -C $LUA_SRC install INSTALL_TOP=$PORTBUILD_PATH
}

function build_jpeg()
{
    if ! [ -d "${JPEG_SRC}/build_${PLATFORM}" ]; then mkdir -p "${JPEG_SRC}/build_${PLATFORM}"; fi
    echo "## JPEG_SRC=$JPEG_SRC"

    export CFLAGS="-m32"
    pushd "${JPEG_SRC}/build_${PLATFORM}"
    ../configure --host=i386-linux-gnu \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM && make install 
    popd
}

function build_bz2()
{
    echo "## BZ2_SRC=$BZ2_SRC"
    
    make -C $BZ2_SRC clean
    make -C $BZ2_SRC all CC="gcc -m32" -j$CORE_NUM
    make -C $BZ2_SRC install PREFIX=$PORTBUILD_PATH
}

function build_sdl2()
{
    if ! [ -d "${SDL2_SRC}/build_${PLATFORM}" ]; then mkdir -p "${SDL2_SRC}/build_${PLATFORM}"; fi
    echo "## SDL2_SRC=$SDL2_SRC"

    export CFLAGS="-Os"
    export CXXFLAGS="-Os"
    pushd "${SDL2_SRC}/build_${PLATFORM}"
    ../configure --host=i686-linux-gnu \
        "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32" \
        --disable-3dnow --disable-sse --disable-sse3 \
        --disable-video-wayland --disable-video-offscreen \
        --enable-video-x11  --enable-x11-shared  \
        --prefix=$PORTBUILD_PATH
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
    ../configure --host=i686-linux-gnu \
        "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32" \
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
    ../configure --host=i686-linux-gnu \
        "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32" \
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
    ../configure --host=i686-linux-gnu \
        "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32" \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM && make install 
    popd
}