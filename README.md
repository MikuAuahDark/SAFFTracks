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

* FFmpeg 4.0 or later, compiled for Windows 32-bit

* CMake 3.1 or later

* GTASA US 1.0 HOODLUM

Ensure you build FFmpeg first, then use CMake to compile the project.

```cmd
rem assume %FFMPEG% contain path where it has "lib" and "include"
cmake -Bbuild -H. -T v141_xp -A Win32 -DLIBAV_LIB_DIR=%FFMPEG%\lib -DLIBAV_INCLUDE_DIR=%FFMPEG%\include -DCMAKE_INSTALL_PREFIX=%CD%\install
cmake --build build --config Release --target install
```

You'll found the ASI plugin in `install`.

Installation
-----

So you have the ASI plugin compiled or downloaded now. Place all the DLLs into your GTASA directory.
Ensure you have ASI loader beforehand.

Special Thanks
-----

Special thanks to Code Nulls Discord for guiding me to reverse engineer the underlying User Tracks
code. This would took very long time without them.
