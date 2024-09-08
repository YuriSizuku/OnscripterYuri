cmake_minimum_required(VERSION 3.16.3)
project(onsyuri)

function(config_linux TARGET_NAME)
    add_executable(${TARGET_NAME} ${ONSYURI_CODE})
    message("Linux enviroment for ${TARGET_NAME}, type ${CMAKE_BUILD_TYPE}, compiler ${CMAKE_C_COMPILER}")
    target_compile_definitions(${TARGET_NAME} PRIVATE
        ${ONSYURI_DEFINES}
        LINUX
        USE_PARALLEL
        USE_GLES
        USE_FILELOG
    )
    if(CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
        target_compile_options(${TARGET_NAME} PRIVATE "-flto")
        target_link_options(${TARGET_NAME} PRIVATE "-s" "-flto")
    endif()

    if(STATIC_PORT_ROOT)
        target_include_directories(${TARGET_NAME} PRIVATE
            ${STATIC_PORT_ROOT}/include
        )
        target_link_directories(${TARGET_NAME} PRIVATE
            ${STATIC_PORT_ROOT}/lib
        )
        target_link_options(${TARGET_NAME} PRIVATE 
            -static-libgcc
            -static-libstdc++
        )
        target_link_libraries(${TARGET_NAME} PRIVATE # static
            -Wl,-Bstatic
            lua
            jpeg
            bz2
            SDL2
            SDL2_ttf
            SDL2_image
            SDL2_mixer
            -Wl,-Bdynamic
            dl
            pthread
        )

    else()
        include(FindPkgConfig)
        pkg_check_modules(SDL2 REQUIRED sdl2)
        pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
        pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
        pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
        pkg_check_modules(JPEG REQUIRED libjpeg)

        target_include_directories(${TARGET_NAME} PRIVATE
            /usr/include/lua5.3
        )
        target_link_libraries(${TARGET_NAME} PRIVATE
            -Wl,-Bstatic # avoid libjpeg.so.8 
            ${JPEG_STATIC_LIBRARIES} 
            bz2
            lua5.3
            -Wl,-Bdynamic
            dl
            ${SDL2_LIBRARIES}
            ${SDL2_TTF_LIBRARIES}
            ${SDL2_IMAGE_LIBRARIES}
            ${SDL2_MIXER_LIBRARIES}
        )
    endif()

    if(CMAKE_SYSROOT) # cross compile
        message("CMAKE_SYSROOT ${CMAKE_SYSROOT}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${CMAKE_SYSROOT}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=${CMAKE_SYSROOT}")
        target_include_directories(${TARGET_NAME} PRIVATE
            ${CMAKE_SYSROOT}/include
        )
        target_link_directories(${TARGET_NAME} PRIVATE
            ${CMAKE_SYSROOT}/lib
        )
    endif()

endfunction()

function(config_darwin TARGET_NAME)
    add_executable(${TARGET_NAME} ${ONSYURI_CODE})
    message("Darwin enviroment for ${TARGET_NAME}, type ${CMAKE_BUILD_TYPE}, compiler ${CMAKE_C_COMPILER}")
    target_compile_definitions(${TARGET_NAME} PRIVATE
            ${ONSYURI_DEFINES}
            MACOSX
            USE_PARALLEL
            USE_GLES
            USE_FILELOG
    )
    target_compile_options(${TARGET_NAME} PRIVATE
            -Wno-deprecated-declarations
            -Wno-invalid-source-encoding
    )

    if(CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
        target_compile_options(${TARGET_NAME} PRIVATE "-flto")
        target_link_options(${TARGET_NAME} PRIVATE "-flto")
    endif()

    include(FindPkgConfig)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
    pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
    pkg_check_modules(LUA REQUIRED lua)

    target_include_directories(${TARGET_NAME} PRIVATE
            ${LUA_INCLUDE_DIRS}
            ${SDL2_INCLUDE_DIRS}
            ${SDL2_TTF_INCLUDE_DIRS}
            ${SDL2_IMAGE_INCLUDE_DIRS}
            ${SDL2_MIXER_INCLUDE_DIRS}
    )
    target_link_libraries(${TARGET_NAME} PRIVATE
            bz2
            ${LUA_LINK_LIBRARIES}
            ${SDL2_LINK_LIBRARIES}
            ${SDL2_TTF_LINK_LIBRARIES}
            ${SDL2_IMAGE_LINK_LIBRARIES}
            ${SDL2_MIXER_LINK_LIBRARIES}
    )
endfunction()

function(config_windows_mingw TARGET_NAME)
    add_executable(${TARGET_NAME} WIN32 
        ${ONSYURI_CODE}
        ${PROJECT_SOURCE_DIR}/asset/icon1.rc)
    if(MINGW)
        message("MINGW enviroment for ${TARGET_NAME}, type ${CMAKE_BUILD_TYPE}")
        if(CMAKE_BUILD_TYPE MATCHES "Debug")
            message("generating debug configuration")
            target_compile_definitions(${TARGET_NAME} PRIVATE _DEBUG)
            target_compile_options(${TARGET_NAME} PRIVATE -g)
            target_link_options(${TARGET_NAME} PRIVATE "-Wl,-subsystem,console") 
        else()
            if(CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
                target_compile_options(${TARGET_NAME} PRIVATE "-flto")
                target_link_options(${TARGET_NAME} PRIVATE "-flto")
            endif()
            target_link_options(${TARGET_NAME} PRIVATE "-Wl,-subsystem,windows")
        endif()

        target_compile_definitions(${TARGET_NAME} PRIVATE
            ${ONSYURI_DEFINES}
            _WIN32
            USE_PARALLEL
            USE_SIMD
            USE_SIMD_X86_SSE2
            USE_GLES
            USE_FILELOG
        )
        target_link_options(${TARGET_NAME} PRIVATE 
            -static-libgcc   # static link mingw env
            -static-libstdc++
        ) 
        target_link_libraries(${TARGET_NAME}  # static links
            -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive 
            -Wl,-Bstatic 
            mingw32 
            SDL2main
            SDL2
            lua

            SDL2_image
            m
            png
            jxl
            hwy
            jpeg
            tiff
            zstd
            deflate
            webp
            sharpyuv
            lzma
            lerc
            jbig

            SDL2_ttf
            harfbuzz
            rpcrt4
            Usp10
            graphite2
            brotlidec
            brotlicommon
            freetype
            z
            bz2
        
            SDL2_mixer
            opusfile
            opus
            ogg
            mpg123
            shlwapi
        )

        target_link_libraries(${TARGET_NAME} 
            -Wl,-Bdynamic # dynamic links
            dinput8 shell32 setupapi advapi32 uuid version oleaut32 ole32 
            imm32 winmm gdi32 user32
        )

    else()
        message("Windows enviroment for ${TARGET_NAME} not supported!")
    endif()
endfunction()

function(config_windows_mingw2 TARGET_NAME)
    add_executable(${TARGET_NAME} WIN32 
        ${ONSYURI_CODE}
        ${PROJECT_SOURCE_DIR}/asset/icon1.rc)
    message("Cross MINGW enviroment for ${TARGET_NAME}, type ${CMAKE_BUILD_TYPE}, compiler ${CMAKE_C_COMPILER}")
   
    # config build type and arch
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        message("generating debug configuration")
        target_compile_definitions(${TARGET_NAME} PRIVATE _DEBUG)
        target_compile_options(${TARGET_NAME} PRIVATE -g)
        target_link_options(${TARGET_NAME} PRIVATE "-Wl,-subsystem,console") 
    else()
        if(CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
            target_compile_options(${TARGET_NAME} PRIVATE "-flto")
            target_link_options(${TARGET_NAME} PRIVATE "-s" "-flto")
        endif()
        target_link_options(${TARGET_NAME} PRIVATE "-Wl,-subsystem,windows")
    endif()
    if(WIN64) 
        set(ARCH x86_64)
    else() 
        set(ARCH i686) 
    endif()
    set(CMAKE_RC_COMPILER "${ARCH}-w64-mingw32-windres")

    # config compile 
    target_compile_definitions(${TARGET_NAME} PRIVATE
        ${ONSYURI_DEFINES}
        _WIN32
        XMD_H # fix jpeg IN32 problem in jmorecfg.h
        USE_PARALLEL
        USE_GLES
        USE_FILELOG
    )
    target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_SYSROOT}/include
        /usr/${ARCH}-w64-mingw32/include
    )
    target_link_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_SYSROOT}/lib
    )
    target_link_options(${TARGET_NAME} PRIVATE 
        -static-libgcc
        -static-libstdc++
    )
    target_link_libraries(${TARGET_NAME} PRIVATE # static
        -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive 
        -Wl,-Bstatic
        lua
        bz2
        
        mingw32
        SDL2
        SDL2main
        SDL2_ttf
        SDL2_image
        SDL2_mixer
        ssp
        
        -Wl,-Bdynamic
        dinput8 shell32 setupapi advapi32 uuid version oleaut32 ole32 
        imm32 winmm gdi32 user32 rpcrt4
    )
endfunction()

function(config_windows_msvc TARGET_NAME)
    add_executable(${TARGET_NAME} WIN32 
        ${ONSYURI_CODE}
        ${PROJECT_SOURCE_DIR}/asset/icon1.rc)
    if(MSVC)
        # get package with cpp package manager like vcpkg
        find_package(Lua REQUIRED)
        find_package(SDL2 CONFIG REQUIRED)
        find_package(SDL2_image CONFIG REQUIRED)
        find_package(SDL2_ttf CONFIG REQUIRED)
        find_package(SDL2_mixer CONFIG REQUIRED)
        find_package(BZip2 REQUIRED)
        find_package(JPEG REQUIRED)
        find_package(Ogg CONFIG REQUIRED)
        find_package(Vorbis CONFIG REQUIRED)

        message("MSVC enviroment for ${TARGET_NAME}, type ${CMAKE_BUILD_TYPE}")
        if(CMAKE_BUILD_TYPE MATCHES "Debug")
            message("generating debug configuration")
            target_compile_definitions(${TARGET_NAME} PRIVATE _DEBUG)
            target_compile_options(${TARGET_NAME} PRIVATE -g)
            target_link_options(${TARGET_NAME} PRIVATE "/SUBSYSTEM:CONSOLE") 
        else()
            target_link_options(${TARGET_NAME} PRIVATE "/SUBSYSTEM:WINDOWS")
        endif()

        target_compile_definitions(${TARGET_NAME} PRIVATE 
            ${ONSYURI_DEFINES}
            _WIN32
            USE_PARALLEL
            USE_GLES
            USE_FILELOG
        )

        # currently (Windows 11), Windows only supports IA32 (x86) / AMD64 / ARM64
        if(${CMAKE_VS_PLATFORM_NAME} STREQUAL "ARM64")
            # ARM64
            message("ARM NEON vector extension is enabled")
            target_compile_definitions(${TARGET_NAME} PRIVATE 
                USE_SIMD
                USE_SIMD_ARM_NEON
            )
        else()
            # x86(-64)
            message("x86 SSE2 vector extension is enabled")
            target_compile_definitions(${TARGET_NAME} PRIVATE 
                USE_SIMD
                USE_SIMD_X86_SSE2
            )
        endif()
        # include
        target_include_directories(${TARGET_NAME} PRIVATE ${LUA_INCLUDE_DIR})
        target_include_directories(${TARGET_NAME} PRIVATE ${JPEG_INCLUDE_DIR})
        # link
        target_link_libraries(${TARGET_NAME} PRIVATE ${JPEG_LIBRARIES})
        target_link_libraries(${TARGET_NAME} PRIVATE Vorbis::vorbis)
        target_link_libraries(${TARGET_NAME} PRIVATE Vorbis::vorbisfile)
        target_link_libraries(${TARGET_NAME} PRIVATE Vorbis::vorbisenc)
        target_link_libraries(${TARGET_NAME} PRIVATE Ogg::ogg)
        target_link_libraries(${TARGET_NAME} PRIVATE ${LUA_LIBRARIES})
        target_link_libraries(${TARGET_NAME}
            PRIVATE
            $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
            $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        )
        target_link_libraries(${TARGET_NAME}
            PRIVATE
            $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
        )
        target_link_libraries(${TARGET_NAME}
            PRIVATE
            $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
        )
        target_link_libraries(${TARGET_NAME}
            PRIVATE
            $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
        )
        target_link_libraries(${TARGET_NAME} PRIVATE BZip2::BZip2)
        
        # use /MT
        set_property(TARGET ${TARGET_NAME} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        message("Windows enviroment for ${TARGET_NAME} not supported!")
    endif()
endfunction()

function(config_web TARGET_NAME)
    add_executable(${TARGET_NAME} ${ONSYURI_CODE})
    message("Web enviroment for "  ${TARGET_NAME})
    target_link_directories(${TARGET_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/thirdparty/build/arch_wasm/lib
    )
    target_include_directories(${TARGET_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/thirdparty/build/arch_wasm/include
    )

    if(CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
        target_compile_options(${TARGET_NAME} PRIVATE "-flto")
        target_link_options(${TARGET_NAME} PRIVATE "-s" "-flto")
    endif()

    # pass flags for emcc
    set(EMCC_FLAGS "-o ${TARGET_NAME}.js -sASYNCIFY -sALLOW_MEMORY_GROWTH=1") # this two is very important! 
    set(EMCC_FLAGS "${EMCC_FLAGS} -sMODULARIZE -sINVOKE_RUN=0 -sEXPORT_NAME=${TARGET_NAME}")
    set(EMCC_FLAGS "${EMCC_FLAGS} -sEXPORTED_RUNTIME_METHODS=callMain,UTF8ToString,FS,PATH,IDBFS") # make as module
    set(EMCC_FLAGS "${EMCC_FLAGS} -sUSE_SDL=2 -sUSE_SDL_TTF=2 -sUSE_BZIP2=1")
    set(EMCC_FLAGS "${EMCC_FLAGS} -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS=bmp,png,jpg")
    set(EMCC_FLAGS "${EMCC_FLAGS} -sUSE_SDL_MIXER=2 -sSDL2_MIXER_FORMATS=ogg,mp3,mid -sUSE_OGG=1 -sUSE_VORBIS=1 -sUSE_MPG123=1")
    set(EMCC_FLAGS "${EMCC_FLAGS} -sFORCE_FILESYSTEM -lidbfs.js --use-preload-plugins")
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS ${EMCC_FLAGS})
    target_compile_options(${TARGET_NAME} PRIVATE 
        -Wno-invalid-source-encoding
    )
    target_compile_definitions(${TARGET_NAME} PRIVATE
        ${ONSYURI_DEFINES}
        WEB
        LINUX
        USE_PARALLEL
    )
    target_link_libraries(${TARGET_NAME} PRIVATE
        lua
    )
endfunction()

function(config_android TARGET_NAME)
    add_library(${TARGET_NAME} SHARED ${ONSYURI_CODE})
    message("Android enviroment for ${TARGET_NAME}, type ${CMAKE_BUILD_TYPE}, compiler ${CMAKE_C_COMPILER}")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        message("generating debug configuration")
        target_compile_definitions(${TARGET_NAME} PRIVATE _DEBUG)
        target_compile_options(${TARGET_NAME} PRIVATE -g)
    endif()

    if(CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
        target_compile_options(${TARGET_NAME} PRIVATE "-flto")
        target_link_options(${TARGET_NAME} PRIVATE "-s" "-flto")
    endif()

    target_compile_definitions(${TARGET_NAME} PRIVATE
        ${ONSYURI_DEFINES} 
        ANDROID
        USE_PARALLEL
        USE_GLES
        USE_FILELOG
        UTF8_FILESYSTEM
        SDL_JAVA_PACKAGE_PATH=com_yuri_onscripter
    )
    target_compile_options(${TARGET_NAME} PRIVATE 
        -Wno-invalid-source-encoding
    )
    target_link_libraries(${TARGET_NAME} PRIVATE
        lua
        jpeg
        bz2
        SDL2
        SDL2_ttf
        SDL2_image
        SDL2_mixer
        
        GLESv2
        log
    )
endfunction()

function(config_platform TARGET_NAME)
    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        config_linux(${TARGET_NAME})
    elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
        if(MSVC)
            config_windows_msvc(${TARGET_NAME})
        else()
            if (CMAKE_SYSROOT) 
                config_windows_mingw2(${TARGET_NAME})
            else() 
                config_windows_mingw(${TARGET_NAME}) 
            endif()
        endif()
    elseif(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
        config_web(${TARGET_NAME})
        add_custom_command(TARGET ${TARGET_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy 
                ${CMAKE_SOURCE_DIR}/src/onsyuri_web/*.html ${CMAKE_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy 
                ${CMAKE_SOURCE_DIR}/src/onsyuri_web/*.py ${CMAKE_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy 
                ${CMAKE_SOURCE_DIR}/asset/icon1.ico ${CMAKE_BINARY_DIR}/favicon.ico
        )
    elseif(CMAKE_SYSTEM_NAME MATCHES "Android")
        config_android(${TARGET_NAME})
    elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
        config_darwin(${TARGET_NAME})
    else()
        message("${CMAKE_SYSTEM_NAME} enviroment for ${TARGET_NAME} not supported!")
    endif()
endfunction()

set(CMAKE_CXX_STANDARD 11)
set(ONSYURI_CODE
    src/onsyuri/AnimationInfo.cpp
    src/onsyuri/coding2utf16.cpp
    src/onsyuri/DirectReader.cpp
    src/onsyuri/DirtyRect.cpp
    src/onsyuri/FontInfo.cpp
    src/onsyuri/gbk2utf16.cpp
    src/onsyuri/LUAHandler.cpp
    src/onsyuri/NsaReader.cpp
    src/onsyuri/ONScripter.cpp
    src/onsyuri/ONScripter_animation.cpp
    src/onsyuri/ONScripter_command.cpp
    src/onsyuri/ONScripter_effect.cpp
    src/onsyuri/ONScripter_effect_breakup.cpp
    src/onsyuri/ONScripter_event.cpp
    src/onsyuri/ONScripter_file.cpp
    src/onsyuri/ONScripter_file2.cpp
    src/onsyuri/ONScripter_image.cpp
    src/onsyuri/ONScripter_lut.cpp
    src/onsyuri/onscripter_main.cpp
    src/onsyuri/ONScripter_rmenu.cpp
    src/onsyuri/ONScripter_sound.cpp
    src/onsyuri/ONScripter_text.cpp
    src/onsyuri/Parallel.cpp
    src/onsyuri/resize_image.cpp
    src/onsyuri/SarReader.cpp
    src/onsyuri/ScriptHandler.cpp
    src/onsyuri/ScriptParser.cpp
    src/onsyuri/ScriptParser_command.cpp
    src/onsyuri/sjis2utf16.cpp
    src/onsyuri/renderer/gles_renderer.cpp
    src/onsyuri/builtin_dll/layer_oldmovie.cpp
    src/onsyuri/builtin_dll/layer_snow.cpp
    src/onsyuri/builtin_dll/ONScripter_effect_cascade.cpp
    src/onsyuri/builtin_dll/ONScripter_effect_trig.cpp
)
set(ONSYURI_DEFINES
    USE_LUA
    USE_BUILTIN_LAYER_EFFECTS
    ENABLE_1BYTE_CHAR
)
config_platform(${PROJECT_NAME})
