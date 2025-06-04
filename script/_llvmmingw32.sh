# must use after _fetch.sh from cross_llvmmingw32.sh

function build_lua()
{
    echo "## LUA_SRC=$LUA_SRC"
    
    make -C $LUA_SRC clean
    make -C $LUA_SRC all PLAT=mingw -j$CORE_NUM \
        CC=i686-w64-mingw32-clang AR="i686-w64-mingw32-ar rcu"
    cp $LUA_SRC/src/lua.exe  $LUA_SRC/src/lua
    cp $LUA_SRC/src/luac.exe  $LUA_SRC/src/luac
    make -C $LUA_SRC install INSTALL_TOP=$PORTBUILD_PATH
}

function build_jpeg()
{
    if ! [ -d "${JPEG_SRC}/build_${PLATFORM}" ]; then mkdir -p "${JPEG_SRC}/build_${PLATFORM}"; fi
    echo "## JPEG_SRC=$JPEG_SRC"
    
    # problems for llvm-mingw
    pushd "${JPEG_SRC}/build_${PLATFORM}"
    ../configure --host=i686-w64-mingw32 \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM && make install 
    popd
}

function build_bz2()
{
    echo "## BZ2_SRC=$BZ2_SRC CC=$CC"
    
    make -C $BZ2_SRC clean
    make -C $BZ2_SRC bzip2 -j$CORE_NUM # this has some problem by mingw
    make -C $BZ2_SRC install PREFIX=$PORTBUILD_PATH
}

function build_sdl2()
{
    if ! [ -d "${SDL2_SRC}/build_${PLATFORM}" ]; then mkdir -p "${SDL2_SRC}/build_${PLATFORM}"; fi
    echo "## SDL2_SRC=$SDL2_SRC"

    export CFLAGS="-Os"
    export CXXFLAGS="-Os"
    pushd "${SDL2_SRC}/build_${PLATFORM}"
    ../configure --host=i686-w64-mingw32 \
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

    pushd "${SDL2_IMAGE_SRC}/build_${PLATFORM}"
    cmake .. -G "Unix Makefiles" \
        -DCMAKE_INSTALL_PREFIX=$PORTBUILD_PATH \
        -DSDL2IMAGE_BACKEND_STB=on \
        -DSDL2IMAGE_SAMPLES=off \
        -DBUILD_SHARED_LIBS=off
    make -j$CORE_NUM &&  make install
    popd
}

# after build_sdl2
function build_sdl2_ttf() 
{
    if ! [ -d "${SDL2_TTF_SRC}/build_${PLATFORM}" ]; then mkdir -p "${SDL2_TTF_SRC}/build_${PLATFORM}"; fi
    echo "## SDL2_TTF_SRC=$SDL2_TTF_SRC"

    # harfbuzz makes very large, using cmake needs -lfreetype
    pushd "${SDL2_TTF_SRC}/build_${PLATFORM}" 
    cmake .. -G "Unix Makefiles" \
        -DCMAKE_INSTALL_PREFIX=$PORTBUILD_PATH \
        -DSDL2TTF_VENDORED=on \
        -DSDL2TTF_HARFBUZZ=off \
        -DSDL2TTF_SAMPLES=off \
        -DBUILD_SHARED_LIBS=off
    make -j$CORE_NUM &&  make install
    popd
}

# after build_sdl2s
function build_sdl2_mixer() 
{
    if ! [ -d "${SDL2_MIXER_SRC}/build_${PLATFORM}" ]; then mkdir -p "${SDL2_MIXER_SRC}/build_${PLATFORM}"; fi
    echo "## SDL2_MIXER_SRC=$SDL2_MIXER_SRC"

    pushd "${SDL2_MIXER_SRC}/build_${PLATFORM}"
    cmake .. -G "Unix Makefiles" \
        -DCMAKE_INSTALL_PREFIX=$PORTBUILD_PATH \
        -DSDL2MIXER_SAMPLES=off \
        -DSDL2MIXER_VORBIS=STB \
        -DSDL2MIXER_OPUS=off \
        -DSDL2MIXER_FLAC=off \
        -DSDL2MIXER_MOD=off \
        -DSDL2MIXER_MIDI=off \
        -DBUILD_SHARED_LIBS=off
    make -j$CORE_NUM &&  make install
    popd
}