cmake_minimum_required(VERSION 3.16.3)
project(OnsJH)

function(config_linux TARGET_NAME)
    message("Linux enviroment for "  ${TARGET_NAME})
    target_compile_definitions(${TARGET_NAME} PRIVATE 
        USE_OPENGL
        _LINUX)
    target_link_libraries(${TARGET_NAME}  # dynamic
        SDL2 
        GL)
endfunction()

function(config_windows TARGET_NAME)
    if(MINGW)
        message("MINGW enviroment for ${TARGET_NAME}, type ${CMAKE_BUILD_TYPE}")
        target_compile_definitions(${TARGET_NAME} PRIVATE 
            _WIN32
            USE_LUA)
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

        target_link_libraries(${TARGET_NAME}   # dynamic links
            -Wl,-Bdynamic
            dinput8 shell32 setupapi advapi32 uuid version oleaut32 ole32 
            imm32 winmm gdi32 user32)

    else()
        message("Windows enviroment for ${TARGET_NAME} not supported!")
    endif()
endfunction()

function(config_web TARGET_NAME)
    message("Web enviroment for "  ${TARGET_NAME})
    
    # pass flags for emcc
    set(EMCC_FLAGS "-sUSE_SDL=2 -o ${TARGET_NAME}.js") 
    if (${TARGET_NAME} MATCHES gl)
        set(EMCC_FLAGS "${EMCC_FLAGS} -sFULL_ES3 -sMIN_WEBGL_VERSION=2")
        set(EMCC_FLAGS "${EMCC_FLAGS} -sFORCE_FILESYSTEM --preload-file=${CMAKE_BINARY_DIR}/assets@assets")
        target_compile_definitions(${TARGET_NAME} PRIVATE USE_OPENGL)
    endif()
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS ${EMCC_FLAGS})
    
    target_include_directories(${TARGET_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/externlib/glm)
    target_compile_definitions(${TARGET_NAME} PRIVATE 
        _WEB
        _LINUX)
    target_link_libraries(${TARGET_NAME}
        SDL2
        GL)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E 
        copy ${CMAKE_SOURCE_DIR}/src/example/*.html ${CMAKE_BINARY_DIR})
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

message("OnsJh multi platform")
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
)

add_executable(${PROJECT_NAME} ${ONSJH_CODE})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src)
config_platform(${PROJECT_NAME})