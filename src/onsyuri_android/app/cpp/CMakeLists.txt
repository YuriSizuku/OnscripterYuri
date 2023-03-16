cmake_minimum_required(VERSION 3.16.3)
project(onsyuri_android)

# configure path
set(ONSYURI_DIR ${CMAKE_SOURCE_DIR}/../../../..)
set(THIRDPARTY_PORT "${ONSYURI_DIR}/thirdparty/port")
set(SDL2_DIR ${THIRDPARTY_PORT}/SDL2-2.26.3)
SET(SDL2IMAGE_DIR ${THIRDPARTY_PORT}/SDL2_image-2.6.3)
set(SDL2TTF_DIR ${THIRDPARTY_PORT}/SDL2_ttf-2.20.2)
set(SDL2MIXER_DIR ${THIRDPARTY_PORT}/SDL2_mixer-2.6.3)
set(JPEG_DIR ${THIRDPARTY_PORT}/SDL2_image-2.6.3/external/jpeg)
set(LUA_DIR ${THIRDPARTY_PORT}/lua-5.4.4)
set(BZ2_DIR ${THIRDPARTY_PORT}/bzip2-1.0.8)

# build sdl2
set(ANDROID TRUE)
set(CMAKE_SYSTEM_NAME Linux)
add_subdirectory(${SDL2_DIR} ${ONSYURI_DIR}/build_android/thirdparty/sdl2)

# build sdl2_image
set(SDL2IMAGE_INSTALL off)
set(SDL2IMAGE_SAMPLES off)
add_subdirectory(${SDL2IMAGE_DIR} ${ONSYURI_DIR}/build_android/thirdparty/sdl2image)

# build sdl2_ttf
set(SDL2TTF_SAMPLES off)
set(SDL2TTF_INSTALL off)
set(SDL2TTF_VENDORED on) # use external ttf in source code
add_subdirectory(${SDL2TTF_DIR} ${ONSYURI_DIR}/build_android/thirdparty/sdl2ttf)

# build sdl2 mixer
set(SDL2MIXER_SAMPLES off)
set(SDL2MIXER_INSTALL off)
set(SDL2MIXER_OPUS off)
set(SDL2MIXER_FLAC off)
set(SDL2MIXER_VORBIS_STB on)
set(SDL2MIXER_VENDORED on)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-dev")
add_subdirectory(${SDL2MIXER_DIR} ${ONSYURI_DIR}/build_android/thirdparty/sdl2mixer)

# build jpeg
add_subdirectory(${JPEG_DIR} ${ONSYURI_DIR}/build_android/thirdparty/jpeg)

# build lua
set(LUASRC_DIR ${LUA_DIR}/src)
set(LUA_CODE
    ${LUASRC_DIR}/lapi.c  ${LUASRC_DIR}/lcode.c ${LUASRC_DIR}/lctype.c ${LUASRC_DIR}/ldebug.c ${LUASRC_DIR}/ldo.c
    ${LUASRC_DIR}/ldump.c ${LUASRC_DIR}/lfunc.c ${LUASRC_DIR}/lgc.c ${LUASRC_DIR}/llex.c ${LUASRC_DIR}/lmem.c
    ${LUASRC_DIR}/lobject.c ${LUASRC_DIR}/lopcodes.c ${LUASRC_DIR}/lparser.c ${LUASRC_DIR}/lstate.c ${LUASRC_DIR}/lstring.c
    ${LUASRC_DIR}/ltable.c ${LUASRC_DIR}/ltm.c  ${LUASRC_DIR}/lundump.c ${LUASRC_DIR}/lvm.c ${LUASRC_DIR}/lzio.c
    ${LUASRC_DIR}/lauxlib.c ${LUASRC_DIR}/lbaselib.c ${LUASRC_DIR}/lcorolib.c ${LUASRC_DIR}/ldblib.c ${LUASRC_DIR}/liolib.c
    ${LUASRC_DIR}/lmathlib.c ${LUASRC_DIR}/loadlib.c ${LUASRC_DIR}/loslib.c  ${LUASRC_DIR}/lstrlib.c ${LUASRC_DIR}/ltablib.c
    ${LUASRC_DIR}/lutf8lib.c ${LUASRC_DIR}/linit.c
)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")
add_library(lua SHARED ${LUA_CODE})

# build bz2
set(BZ2SRC_DIR ${BZ2_DIR})
set(BZ2_CODE
    ${BZ2SRC_DIR}/blocksort.c ${BZ2SRC_DIR}/huffman.c ${BZ2SRC_DIR}/crctable.c ${BZ2SRC_DIR}/randtable.c
    ${BZ2SRC_DIR}/compress.c  ${BZ2SRC_DIR}/decompress.c ${BZ2SRC_DIR}/bzlib.c
)
add_library(bz2 SHARED ${BZ2_CODE})
target_compile_options(bz2 PRIVATE -D_FILE_OFFSET_BITS=64)

# build onsyuri_android
set(CMAKE_SYSTEM_NAME Android)
#set(CMAKE_BUILD_TYPE Debug)
include_directories(
    ${BZ2_DIR}
    ${LUASRC_DIR}
)
add_subdirectory(${ONSYURI_DIR} ${ONSYURI_DIR}/build_android)