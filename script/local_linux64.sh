# bash -c "BUILD_TYPE=Debug OBTAIN_DEPENDS=yes ./local_linux64.sh"
PLATFORM=linux64
BUILD_PATH=./../build_${PLATFORM}
CMAKELISTS_PATH=$(pwd)/..
PORTBUILD_PATH=$CMAKELISTS_PATH/thirdparty/build/arch_$PLATFORM
CORE_NUM=$(cat /proc/cpuinfo | grep -c ^processor)
TARGETS=$@

# config env
CC=gcc
CXX=g++
if [ -z "$BUILD_TYPE" ]; then BUILD_TYPE=MinSizeRel; fi
if [ -z "$TARGETS" ]; then TARGETS=all; fi

# ports from debian
if [ -n "$OBTAIN_DEPENDS" ]; then
    sudo apt-get update
    sudo apt-get -y install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev libsdl2-mixer-dev
    sudo apt-get -y install libbz2-dev libjpeg-dev libpng-dev
    sudo apt-get -y install liblua5.3-dev libgl1-mesa-dev
fi

# config and build project
echo "BUILD_TYPE=$BUILD_TYPE"

cmake -B $BUILD_PATH -S $CMAKELISTS_PATH \
    -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX \
    -DCMAKE_C_FLAGS=-m64 -DCMAKE_CXX_FLAGS=-m64

make -C $BUILD_PATH $TARGETS -j$CORE_NUM