# bash -c "BUILD_TYPE=Debug OBTAIN_DEPENDS=yes ./local_linux32.sh"
PLATFORM=linux32
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
    sudo dpkg --add-architecture i386 
    sudo apt-get update
    sudo apt-get -y install gcc-multilib g++-multilib 
    sudo apt-get -y install libsdl2-dev:i386 libsdl2-ttf-dev:i386 libsdl2-image-dev:i386 libsdl2-mixer-dev:i386
    sudo apt-get -y install libbz2-dev:i386 libjpeg-dev:i386 libpng-dev:i386
    sudo apt-get -y install liblua5.3-dev:i386 libgl1-mesa-dev:i386
fi

# config and build project
echo "BUILD_TYPE=$BUILD_TYPE"

cmake -B $BUILD_PATH -S $CMAKELISTS_PATH \
    -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX \
    -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32

make -C $BUILD_PATH $TARGETS -j$CORE_NUM