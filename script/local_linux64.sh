BUILD_PATH=./../build_linux32
CMAKELISTS_PATH=./../

# prepare libs
if ! [ -d ./../externlib ]; then mkdir ./../externlib; fi
if ! [ -d ./../externlib/stb ]; then git clone https://github.com/nothings/stb.git ./../externlib/stb; fi

# config and build project
cmake -B $BUILD_PATH -S $CMAKELISTS_PATH \
      -G "Unix Makefiles" \
      -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ \
      -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32 
make -C $BUILD_PATH all