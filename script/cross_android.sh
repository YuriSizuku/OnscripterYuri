# bash -c "export SKIP_PORTS=yes && ./cross_android.sh"
PLATFORM=android
BUILD_PATH=./../build_${PLATFORM}
CMAKELISTS_PATH=$(pwd)/..
PORTBUILD_PATH=$CMAKELISTS_PATH/thirdparty/build/arch_$PLATFORM
CORE_NUM=$(cat /proc/cpuinfo | grep -c ^processor)
TARGETS=$@

# prepare_port portname
function prepare_port()
{
    _name=$(basename "$1")
    if [ -f "$1/external/download.sh" ]; then
        chmod +x "$1/external/download.sh"
        "$1/external/download.sh"
    fi
}

# SKIP_PORTS="yes"
if [ -z "$SKIP_PORTS" ]; then
    source ./_fetch.sh
    source ./_$PLATFORM.sh
    fetch_bz2 ; prepare_port $BZ2_SRC
    fetch_lua ; prepare_port $LUA_SRC
    fetch_sdl2 ; prepare_port $SDL2_SRC
    fetch_sdl2_image ; prepare_port $SDL2_IMAGE_SRC
    fetch_sdl2_ttf ; prepare_port $SDL2_TTF_SRC
    fetch_sdl2_mixer ; prepare_port $SDL2_MIXER_SRC
fi

# config and build project
if [ -z "$BUILD_TYPE" ]; then BUILD_TYPE=MinSizeRel; fi
if [ -z "$TARGETS" ]; then TARGETS=assembleRelease; fi

pushd ${CMAKELISTS_PATH}/src/onsyuri_android
echo "ANDROID_HOME=$ANDROID_HOME" 
chmod +x ./gradlew && ./gradlew $TARGETS --no-daemon
popd