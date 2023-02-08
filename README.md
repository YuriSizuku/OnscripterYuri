# Onscripter (Yuri)  

☘️ An ehancement Onscripter project porting to many platforms including web.
This is base on [ONScripter-Jh](https://github.com/jh10001/ONScripter-Jh) by `SDL2`.

Features:  

- [x] clear camke project for multi platforms  
  - [x] mingw32 in windows
  - [ ] linux x64
  - [ ] linux aarch64 (raspberrypi)
  - [ ] web
  - [ ] android
  - [ ] psv, see [psv-Onscripter](https://github.com/YuriSizuku/psv-OnscripterJH)
- [ ] ci in github action to automaticly build  
- [x] support `nt2`, `nt3` encryption format by Mine  
- [x] support fullscreen by `--fullscreen` or `alt+enter`, scretch to fullscreen by `--fullscreen2` or `f11`  
- [x] support arbitary resolution `--width`, `--height`  
- [x] fix some bugs in origin version (can not read `00.txt` problem)  

## 1. usage

## (1) command

``` bash
./ons_yuri --help
./ons_yuri --root /path/to/game --save-dir /path/to/save --font /path/default.ttf
./ons_yuri --width 1280 --height 720 --enc:sjis
./ons_yuri --fullscreen2 # fullscreen1 alt+f4, fullscreen2 f11
```

❗ If you force exit the game, the save might be damaged, try to remvoe envdata to play again.

## (2) web

## 2. build

### (1) local mingw  

Install the enviroment in msys

``` sh
pacman -Syu --noconfirm
pacman -S --noconfirm make tar vim curl # util tools
pacman -S --noconfirm mingw-w64-x86_64-binutils mingw-w64-x86_64-gcc mingw-w64-x86_64-gdb # mingw64 compile tool
pacman -S --noconfirm mingw-w64-i686-binutils mingw-w64-i686-gcc mingw-w64-i686-gdb # mingw32 compile tool

pacman -S --noconfirm mingw-w64-i686-SDL2 mingw-w64-x86_64-SDL2
pacman -S --noconfirm mingw-w64-i686-SDL2_image mingw-w64-x86_64-SDL2_image
pacman -S --noconfirm mingw-w64-i686-SDL2_ttf mingw-w64-x86_64-SDL2_ttf
pacman -S --noconfirm mingw-w64-i686-SDL2_mixer mingw-w64-x86_64-SDL2_mixer

pacman -S --noconfirm mingw-w64-i686-brotli mingw-w64-x86_64-brotli
pacman -S --noconfirm mingw-w64-i686-mesa mingw-w64-x86_64-mesa
pacman -S --noconfirm mingw-w64-i686-lua mingw-w64-x86_64-lua
```

and then use these `local_mingw32.sh` to build.

``` sh
cd script
sh -c "export BUILD_TYPE=Debug && export MSYS2SDK=/path/to/msys2 && ./local_mingw32.sh"
```

### (2) local linux x64

### (3) cross web  
