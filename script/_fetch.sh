# prepare dirs
if ! [ -d $CMAKELISTS_PATH/thirdparty/port ]; then mkdir -p $CMAKELISTS_PATH/thirdparty/port; fi
if ! [ -d $CMAKELISTS_PATH/thirdparty/build/arch_mingw32 ]; then mkdir -p $CMAKELISTS_PATH/thirdparty/build/arch_mingw32; fi
if ! [ -d $CMAKELISTS_PATH/thirdparty/build/arch_mingw64 ]; then mkdir -p $CMAKELISTS_PATH/thirdparty/build/arch_mingw64; fi
if ! [ -d $CMAKELISTS_PATH/thirdparty/build/arch_linux32 ]; then mkdir -p $CMAKELISTS_PATH/thirdparty/build/arch_linux32; fi
if ! [ -d $CMAKELISTS_PATH/thirdparty/build/arch_linux64 ]; then mkdir -p $CMAKELISTS_PATH/thirdparty/build/arch_linux64; fi
if ! [ -d $CMAKELISTS_PATH/thirdparty/build/arch_linuxa64 ]; then mkdir -p $CMAKELISTS_PATH/thirdparty/build/arch_linuxa64; fi
if ! [ -d $CMAKELISTS_PATH/thirdparty/build/arch_wasm ]; then mkdir -p $CMAKELISTS_PATH/thirdparty/build/arch_wasm; fi

# urlbase, name, outpath
function fetch_port()
{
    if ! [ -d "$CMAKELISTS_PATH/thirdparty/port/$2" ]; then
        echo "## fetch_port $1 $2"
        curl -fsSL $1/$2.tar.gz -o $CMAKELISTS_PATH/thirdparty/port/$2.tar.gz 
        tar zxf $CMAKELISTS_PATH/thirdparty/port/$2.tar.gz -C $CMAKELISTS_PATH/thirdparty/port
    fi
}

# fetch by curl
function fetch_lua()
{
    LUA_NAME=lua-5.4.4
    LUA_SRC=$CMAKELISTS_PATH/thirdparty/port/$LUA_NAME
    fetch_port http://www.lua.org/ftp $LUA_NAME
}

function fetch_jpeg()
{
    JPEG_NAME=jpeg-9
    JPEG_SRC=$CMAKELISTS_PATH/thirdparty/port/$JPEG_NAME
    fetch_port http://www.ijg.org/files jpegsrc.v9
}

function fetch_bz2()
{  
    BZ2_NAME=bzip2-1.0.8
    BZ2_SRC=$CMAKELISTS_PATH/thirdparty/port/$BZ2_NAME
    fetch_port https://sourceware.org/pub/bzip2 $BZ2_NAME
}

function fetch_sdl2() 
{
    SDL2_NAME=SDL2-2.26.3
    SDL2_SRC=$CMAKELISTS_PATH/thirdparty/port/$SDL2_NAME
    fetch_port https://github.com/libsdl-org/SDL/releases/download/release-2.26.3  $SDL2_NAME
}

function fetch_sdl2_image()
{
    SDL2_IMAGE_NAME=SDL2_image-2.6.3
    SDL2_IMAGE_SRC=$CMAKELISTS_PATH/thirdparty/port/$SDL2_IMAGE_NAME
    fetch_port https://github.com/libsdl-org/SDL_image/releases/download/release-2.6.3 $SDL2_IMAGE_NAME
}

function fetch_sdl2_ttf()
{
    SDL2_TTF_NAME=SDL2_ttf-2.20.2
    SDL2_TTF_SRC=$CMAKELISTS_PATH/thirdparty/port/$SDL2_TTF_NAME
    fetch_port https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.20.2 $SDL2_TTF_NAME
}

function fetch_sdl2_mixer()
{
    SDL2_MIXER_NAME=SDL2_mixer-2.6.3
    SDL2_MIXER_SRC=$CMAKELISTS_PATH/thirdparty/port/$SDL2_MIXER_NAME
    fetch_port https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.6.3 $SDL2_MIXER_NAME
}

# fetch unused port
function fetch_stb()
{
    STB_NAME=stb
    STB_SRC=$CMAKELISTS_PATH/thirdparty/port/$STB_NAME
    git clone https://github.com/nothings/stb.git $STB_SRC
}

function fetch_pulse()
{
    PULSE_NAME=pulseaudio-16.1
    PULSE_SRC=$CMAKELISTS_PATH/thirdparty/port/$PULSE_NAME
    fetch_port https://freedesktop.org/software/pulseaudio/releases/ $PULSE_NAME
}