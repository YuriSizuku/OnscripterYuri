# bash -c "export BUILD_TYPE=Debug && export SKIP_PORTS=1 && ./cross_linuxa64.sh"
PLATFORM=android
BUILD_PATH=./../build_${PLATFORM}
CMAKELISTS_PATH=$(pwd)/..
PORTBUILD_PATH=$CMAKELISTS_PATH/thirdparty/build/arch_$PLATFORM
ANDROID_THIRDPARTY_PATH=$CMAKELISTS_PATH/src/onsyuri_android/app/cpp/thirdparty
CORE_NUM=$(cat /proc/cpuinfo | grep -c ^processor)
TARGETS=$@

# copy_port portname
function copy_port()
{
    _name=$(basename "$1")
    rm -rf $ANDROID_THIRDPARTY_PATH/$_name
    cp -rf $1  $ANDROID_THIRDPARTY_PATH/
    if [ -f "$ANDROID_THIRDPARTY_PATH/$_name/external/download.sh" ]; then
        chmod +x $ANDROID_THIRDPARTY_PATH/$_name/external/download.sh
        $ANDROID_THIRDPARTY_PATH/$_name/external/download.sh
    fi
}

# SKIP_PORTS="yes"
if [ -z "$SKIP_PORTS" ]; then
    source ./_fetch.sh
    source ./_$PLATFORM.sh
    if ! [ -d "$ANDROID_THIRDPARTY_PATH" ]; then mkdir -p $ANDROID_THIRDPARTY_PATH; fi
    fetch_bz2 && copy_port $BZ2_SRC
    fetch_lua && copy_port $LUA_SRC
    fetch_sdl2 && copy_port $SDL2_SRC
    fetch_sdl2_image && copy_port $SDL2_IMAGE_SRC
    fetch_sdl2_ttf && copy_port $SDL2_TTF_SRC
    fetch_sdl2_mixer && copy_port $SDL2_MIXER_SRC
fi

# config and build project
if [ -z "$BUILD_TYPE" ]; then BUILD_TYPE=MinSizeRel; fi
if [ -z "$TARGETS" ]; then TARGETS=all; fi