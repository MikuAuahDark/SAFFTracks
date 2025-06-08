SAFFTracks
======

Do you ever struggle of re-encoding your music to MP3 before putting it to GTA San Andreas?
With this plugin, say goodbye to re-encoding frustration.

This plugin replaces the User Tracks decoder engine to [FFmpeg](https://www.ffmpeg.org/)
which supports lots of audio formats. You can simply drop your songs to User Tracks folder
and this plugin will make sure to load your audio using FFmpeg instead of GTASA's own.
Opus audio? AAC? Other non-mainstream audio formats? This plugin can play those as long as
FFmpeg supports it.

Building
-----

Requirements:

* FFmpeg 7.0 or later, compiled for Windows 32-bit

* CMake 3.1 or later

* GTASA US 1.0 HOODLUM/Compact

Ensure you build FFmpeg first (see below), then use CMake to compile the project.

```cmd
rem assume %FFMPEG% contain path where it has "lib" and "include"
cmake -Bbuild -s. --install-prefix %CD%\install -A Win32 -DLIBAV_LIB_DIR=%FFMPEG%\lib -DLIBAV_INCLUDE_DIR=%FFMPEG%\include
cmake --build build --config RelWithDebInfo --target install
```

You'll found the ASI plugin in `install`.

Protip: You can also specify your GTASA installation directory for the `--install-prefix` option.

Installation
-----

So you have the ASI plugin compiled or downloaded now. Place all the DLLs into your GTASA directory.
Ensure you have ASI loader beforehand.

Building FFmpeg
-----

Notes: `>` indicates CMD shell, `$` indicates Bash shell.

1. Install MSYS2 (really, you can't workaround it with WSL anymore)
2. Modify MSYS2 `ucrt64.ini` such that it inherit the environment variables.
3. Open UCRT64 shell
4. `$ pacman -S make mingw-w64-ucrt-x86_64-nasm`
5. `$ cmd.exe`. You'll get into CMD shell.
6. `> call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsamd64_x86.bat"`
7. `> cd Path\To\FFmpeg7`
8. `> bash`
9. Use the configure command-line below.
```
src/configure --disable-doc --disable-static --enable-shared --disable-programs --disable-avdevice --disable-swscale --disable-postproc --disable-avfilter --disable-network --disable-encoders --disable-muxers --disable-decoder=h264 --disable-decoder=hevc --disable-decoder=mpeg4 --disable-decoder=vp9 --disable-decoder=vp8 --disable-decoder=av1 --disable-decoder=ffv1 --disable-decoder=wmv1 --disable-decoder=wmv2 --disable-decoder=wmv3 --disable-decoder=vvc --disable-decoder=theora --disable-hwaccels --enable-version3 --prefix=$PWD/installdir --toolchain=msvc
```
10. `$ make install -j`
11. You'll have FFmpeg 7 at `Path/To/FFmpeg7/installdir`. Point this to SAFFTracks as your `%FFMPEG%` variable (see above).

Special Thanks
-----

Special thanks to Code Nulls Discord for guiding me to reverse engineer the underlying User Tracks
code. This would took very long time without them.
