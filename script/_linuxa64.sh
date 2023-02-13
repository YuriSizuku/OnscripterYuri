# must use after _fetch.sh from cross_linuxa64.sh

function build_lua()
{
    if ! [ -d "${LUA_SRC}_${PLATFORM}" ]; then cp -rp ${LUA_SRC} "${LUA_SRC}_${PLATFORM}"; fi
    LUA_SRC=${LUA_SRC}_${PLATFORM}
    echo "## LUA_SRC=$LUA_SRC"
    make -C $LUA_SRC all PLAT=linux CC=aarch64-linux-gnu-gcc AR="aarch64-linux-gnu-ar rcu" -j$CORE_NUM
    make -C $LUA_SRC install INSTALL_TOP=$PORTBUILD_PATH
}

function build_jpeg()
{
    if ! [ -d "${JPEG_SRC}_${PLATFORM}" ]; then cp -rp ${JPEG_SRC} "${JPEG_SRC}_${PLATFORM}"; fi
    JPEG_SRC=${JPEG_SRC}_${PLATFORM}
    echo "## SDL2_SRC=$JPEG_SRC"
    pushd $JPEG_SRC
    ./configure --host=aarch64-linux-gnu \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM && make install 
    popd
}

function build_bz2()
{
    if ! [ -d "${BZ2_SRC}_${PLATFORM}" ]; then cp -rp ${BZ2_SRC} "${BZ2_SRC}_${PLATFORM}"; fi
    BZ2_SRC=${BZ2_SRC}_${PLATFORM}
    echo "## BZ2_SRC=$BZ2_SRC"
    make -C $BZ2_SRC all CC=aarch64-linux-gnu-gcc AR=aarch64-linux-gnu-ar -j$CORE_NUM
    make -C $BZ2_SRC install PREFIX=$PORTBUILD_PATH
}

function build_pulse()
{
    # too many dependency for cross compile pulse
    if ! [ -d "${PULSE_SRC}_${PLATFORM}" ]; then cp -rp ${PULSE_SRC} "${PULSE_SRC}_${PLATFORM}"; fi
    PULSE_SRC=${PULSE_SRC}_${PLATFORM}
    echo "## PULSE_SRC=$PULSE_SRC"    
}

function get_pulse()
{
    # get libpulse.so, libpulse-simple.so from arm64 deb
    curl -fsSL http://ports.ubuntu.com/pool/main/p/pulseaudio/libpulse0_13.99.1-1ubuntu3.13_arm64.deb \
        -o $CMAKELISTS_PATH/thirdparty/port/libpulse0_13.99.1-1ubuntu3.13_arm64.deb
    pushd $CMAKELISTS_PATH/thirdparty/port
    ar vx libpulse0_13.99.1-1ubuntu3.13_arm64.deb data.tar.xz
    if ! [ -d "$(pwd)/libpulse_${PLATFORM}" ]; then mkdir libpulse_${PLATFORM}; fi
    tar xf $CMAKELISTS_PATH/thirdparty/port/data.tar.xz -C ./libpulse_${PLATFORM}
    cp -f ./libpulse_${PLATFORM}/usr/lib/aarch64-linux-gnu/libpulse.so.0.21.2 $PORTBUILD_PATH/lib/libpulse.so
    chmod +x $PORTBUILD_PATH/lib/libpulse.so
    cp -f ./libpulse_${PLATFORM}/usr/lib/aarch64-linux-gnu/libpulse-simple.so.0.1.1 $PORTBUILD_PATH/lib/libpulse-simple.so
    chmod +x $PORTBUILD_PATH/lib/libpulse-simple.so
    cp -f ./libpulse_${PLATFORM}/usr/lib/aarch64-linux-gnu/pulseaudio/libpulsecommon-13.99.so $PORTBUILD_PATH/lib/libpulsecommon.so
    chmod +x $PORTBUILD_PATH/lib/libpulsecommon.so
    rm -rf data.tar.xz
    popd
}

function build_sdl2() # after pulse
{
    if ! [ -d "${SDL2_SRC}_${PLATFORM}" ]; then cp -rp ${SDL2_SRC} "${SDL2_SRC}_${PLATFORM}"; fi
    SDL2_SRC=${SDL2_SRC}_${PLATFORM}
    echo "## SDL2_SRC=$SDL2_SRC"
    pushd $SDL2_SRC
    export LDFLAGS="-L$PORTBUILD_PATH/lib"
    SDL2_SYSROOT=/
    if [ -n "$SYSROOT" ]; then SDL2_SYSROOT=$SYSROOT; fi
    echo "## SDL2_SYSROOT $SDL2_SYSROOT"
    ./configure --host=aarch64-linux-gnu \
        --disable-pulseaudio \
        --enable-video-x11  --enable-x11-shared  --enable-video-x11-xcursor --enable-video-x11-xinput --enable-video-x11-xrandr \
        --disable-video-wayland \
        --enable-arm-simd --enable-arm-neon \
        --prefix=$PORTBUILD_PATH --with-sysroot=$SDL2_SYSROOT
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
    ./configure --host=aarch64-linux-gnu \
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
    ./configure --host=aarch64-linux-gnu \
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
    ./configure --host=aarch64-linux-gnu \
        --prefix=$PORTBUILD_PATH
    make -j$CORE_NUM 
    make install 
    popd
}