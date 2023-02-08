cmake_minimum_required(VERSION 3.16.3)
project(ons_yuri)

function(config_linux TARGET_NAME)
    add_executable(${TARGET_NAME} ${ONSJH_CODE})
    message("Linux enviroment for "  ${TARGET_NAME})
    target_compile_definitions(${TARGET_NAME} PRIVATE 
        USE_OPENGL
        _LINUX)
    target_link_libraries(${TARGET_NAME}  # dynamic
        SDL2 
        GL)
endfunction()

function(config_windows TARGET_NAME)
    add_executable(${TARGET_NAME} WIN32 
        ${ONSJH_CODE}
        ${PROJECT_SOURCE_DIR}/asset/icon1.rc)
    if(MINGW)
        message("MINGW enviroment for ${TARGET_NAME}, type ${CMAKE_BUILD_TYPE}")
        target_compile_definitions(${TARGET_NAME} PRIVATE 
            _WIN32
            USE_LUA
            USE_PARALLEL
            USE_SIMD
            USE_SIMD_X86_SSE2
            USE_BUILTIN_LAYER_EFFECTS
            USE_GLES
            USE_FILELOG)
        target_link_options(${TARGET_NAME} PRIVATE 
            -static-libgcc   # static link mingw env
            -static-libstdc++) 
        if(CMAKE_BUILD_TYPE MATCHES "Debug")
            message("generating debug configuration")
            target_compile_definitions(${TARGET_NAME} PRIVATE _DEBUG)
            target_compile_options(${TARGET_NAME} PRIVATE -g)
            target_link_options(${TARGET_NAME} PRIVATE "-Wl,-subsystem,console") 
        else()
            target_link_options(${TARGET_NAME} PRIVATE "-Wl,-subsystem,windows")
        endif()
        target_link_libraries(${TARGET_NAME}  # static links
            -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive 
            -Wl,-Bstatic 
            mingw32 
            SDL2main
            SDL2
            opengl32
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
            shlwapi)

        target_link_libraries(${TARGET_NAME} 
            -Wl,-Bdynamic # dynamic links
            dinput8 shell32 setupapi advapi32 uuid version oleaut32 ole32 
            imm32 winmm gdi32 user32)

    else()
        message("Windows enviroment for ${TARGET_NAME} not supported!")
    endif()
endfunction()

function(config_web TARGET_NAME)
    add_executable(${TARGET_NAME} ${ONSJH_CODE})
    message("Web enviroment for "  ${TARGET_NAME})
    
    # pass flags for emcc
    set(EMCC_FLAGS "-o ${TARGET_NAME}.js -sASYNCIFY -sALLOW_MEMORY_GROWTH=1") # this two is very important! 
    set(EMCC_FLAGS "${EMCC_FLAGS} -sUSE_SDL=2 -sUSE_SDL_TTF=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS=bmp,png,jpg")
    set(EMCC_FLAGS "${EMCC_FLAGS} -sUSE_SDL_MIXER=2 -sUSE_BZIP2=1 -sSDL2_MIXER_FORMATS=ogg,mp3 -sUSE_OGG=1 -sUSE_VORBIS=1")
    set(EMCC_FLAGS "${EMCC_FLAGS} -sFORCE_FILESYSTEM --use-preload-plugins --preload-file=${PROJECT_SOURCE_DIR}/asset/test@/ons_yuri")
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS ${EMCC_FLAGS})
    target_compile_options(${TARGET_NAME} PRIVATE 
        -Wno-invalid-source-encoding
    )
    target_compile_definitions(${TARGET_NAME} PRIVATE 
        WEB
        LINUX
        USE_PARALLEL
        USE_BUILTIN_LAYER_EFFECTS)
    add_custom_command(TARGET ${TARGET_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E 
        copy ${CMAKE_SOURCE_DIR}/src/onsjh_web/*.html ${CMAKE_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E 
        copy ${CMAKE_SOURCE_DIR}/asset/icon1.ico ${CMAKE_BINARY_DIR}/favicon.ico)
endfunction()

function(config_platform TARGET_NAME)
    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        config_linux(${TARGET_NAME})
    elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
        config_windows(${TARGET_NAME})
    elseif(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
        config_web(${TARGET_NAME})
    else()
        message("${CMAKE_SYSTEM_NAME} enviroment for ${TARGET_NAME} not supported!")
    endif()
endfunction()

set(CMAKE_CXX_STANDARD 11)
set(ONSJH_CODE
    src/onsjh/AnimationInfo.cpp
    src/onsjh/coding2utf16.cpp
    src/onsjh/conv_shared.cpp
    src/onsjh/DirectReader.cpp
    src/onsjh/DirtyRect.cpp
    src/onsjh/FontInfo.cpp
    src/onsjh/gbk2utf16.cpp
    src/onsjh/LUAHandler.cpp
    src/onsjh/NsaReader.cpp
    src/onsjh/ONScripter.cpp
    src/onsjh/ONScripter_animation.cpp
    src/onsjh/ONScripter_command.cpp
    src/onsjh/ONScripter_effect.cpp
    src/onsjh/ONScripter_effect_breakup.cpp
    src/onsjh/ONScripter_event.cpp
    src/onsjh/ONScripter_file.cpp
    src/onsjh/ONScripter_file2.cpp
    src/onsjh/ONScripter_image.cpp
    src/onsjh/ONScripter_lut.cpp
    src/onsjh/onscripter_main.cpp
    src/onsjh/ONScripter_rmenu.cpp
    src/onsjh/ONScripter_sound.cpp
    src/onsjh/ONScripter_text.cpp
    src/onsjh/Parallel.cpp
    src/onsjh/resize_image.cpp
    src/onsjh/SarReader.cpp
    src/onsjh/ScriptHandler.cpp
    src/onsjh/ScriptParser.cpp
    src/onsjh/ScriptParser_command.cpp
    src/onsjh/sjis2utf16.cpp
    src/onsjh/renderer/gles_renderer.cpp
    src/onsjh/builtin_dll/layer_oldmovie.cpp
    src/onsjh/builtin_dll/layer_snow.cpp
    src/onsjh/builtin_dll/ONScripter_effect_cascade.cpp
    src/onsjh/builtin_dll/ONScripter_effect_trig.cpp
)
config_platform(${PROJECT_NAME})