# prepare dirs
if ! [ -d $CMAKELISTS_PATH/thirdparty/port ]; then mkdir -p $CMAKELISTS_PATH/thirdparty/port; fi
if ! [ -d $CMAKELISTS_PATH/thirdparty/build/arch_mingw32 ]; then mkdir -p $CMAKELISTS_PATH/thirdparty/build/arch_mingw32; fi
if ! [ -d $CMAKELISTS_PATH/thirdparty/build/arch_linux64 ]; then mkdir -p $CMAKELISTS_PATH/thirdparty/build/arch_linux64; fi
if ! [ -d $CMAKELISTS_PATH/thirdparty/build/arch_linuxa64 ]; then mkdir -p $CMAKELISTS_PATH/thirdparty/build/arch_linuxa64; fi
if ! [ -d $CMAKELISTS_PATH/thirdparty/build/arch_wasm ]; then mkdir -p $CMAKELISTS_PATH/thirdparty/build/arch_wasm; fi

# prepare lua
LUA_NAME=$(ls $CMAKELISTS_PATH/thirdparty/port | grep lua- | grep -v .gz)
if ! [ -n "$LUA_NAME" ]; then
    curl --output-dir $CMAKELISTS_PATH/thirdparty/port -R -O http://www.lua.org/ftp/lua-5.4.4.tar.gz
    tar zxf $CMAKELISTS_PATH/thirdparty/port/lua-5.4.4.tar.gz -C $CMAKELISTS_PATH/thirdparty/port
    LUA_NAME=$(ls $CMAKELISTS_PATH/thirdparty/port | grep lua- | grep -v .gz)
fi
LUA_SRC=$CMAKELISTS_PATH/thirdparty/port/$LUA_NAME