# must use after _fetch.sh from cross_mingw64.sh

function build_lua()
{
    echo "## LUA_SRC=$LUA_SRC"
    
    make -C $LUA_SRC clean
    make -C $LUA_SRC all PLAT=mingw -j$CORE_NUM \
        CC=x86_64-w64-mingw32-gcc AR="x86_64-w64-mingw32-ar rcu"
    cp $LUA_SRC/src/lua.exe  $LUA_SRC/src/lua
    cp $LUA_SRC/src/luac.exe  $LUA_SRC/src/luac
    make -C $LUA_SRC install INSTALL_TOP=$PORTBUILD_PATH
}

function build_jpeg()
{
    if ! [ -d "${JPEG_SRC}/build_${PLATFORM}" ]; then mkdir -p "${JPEG_SRC}/build_${PLATFORM}"; fi
    echo "## JPEG_SRC=$JPEG_SRC"

    pushd "${JPEG_SRC}/build_${PLATFORM}"
    ../configure --host=x86_64-w64-mingw32 \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM && make install 
    popd
}

function build_bz2()
{
    echo "## BZ2_SRC=$BZ2_SRC CC=$CC"
    
    make -C $BZ2_SRC clean
    make -C $BZ2_SRC -j$CORE_NUM # this has some problem by mingw
    make -C $BZ2_SRC install PREFIX=$PORTBUILD_PATH

    pushd $CMAKELISTS_PATH/thirdparty/port/
    curl -fsSL https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-bzip2-1.0.8-2-any.pkg.tar.zst -O
    tar xf mingw-w64-x86_64-bzip2-1.0.8-2-any.pkg.tar.zst mingw64/lib/libbz2.a
    cp mingw64/lib/libbz2.a $PORTBUILD_PATH/lib/
    rm -rf mingw64
    popd
}


function build_sdl2()
{
    if ! [ -d "${SDL2_SRC}/build_${PLATFORM}" ]; then mkdir -p "${SDL2_SRC}/build_${PLATFORM}"; fi
    echo "## SDL2_SRC=$SDL2_SRC"

    export CFLAGS="-Os"
    export CXXFLAGS="-Os"
    pushd "${SDL2_SRC}/build_${PLATFORM}"
    ../configure --host=x86_64-w64-mingw32 \
        --disable-3dnow --disable-sse --disable-sse3 \
        --disable-video-vulkan --disable-video-offscreen \
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
    # stb_image.h already included
    pushd "${SDL2_IMAGE_SRC}/build_${PLATFORM}"
    ../configure --host=x86_64-w64-mingw32 \
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
    ../configure --host=x86_64-w64-mingw32 \
        --disable-harfbuzz \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM && make install 
    popd
}

# after build_sdl2s
function build_sdl2_mixer() 
{
    if ! [ -d "${SDL2_MIXER_SRC}/build_${PLATFORM}" ]; then mkdir -p "${SDL2_MIXER_SRC}/build_${PLATFORM}"; fi
    echo "## SDL2_MIXER_SRC=$SDL2_MIXER_SRC"

    export PKG_CONFIG_PATH=${PORTBUILD_PATH}/lib/pkgconfig # this is important for find SDL path    
    pushd "${SDL2_MIXER_SRC}/build_${PLATFORM}"
    ../configure --host=x86_64-w64-mingw32 \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM && make install 
    popd
}