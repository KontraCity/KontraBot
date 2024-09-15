# KontraBot
Yet another music bot for Discord that can play videos and playlists from YouTube!
Supports multiple languages, video chapters and shows playing items in voice channel status.

## Build
#### Dependencies
* [libfmt](https://github.com/fmtlib/fmt)
* [libspdlog](https://github.com/gabime/spdlog)
* [libcurl](https://github.com/curl/curl)
* [libdpp](https://github.com/brainboxdotcc/DPP)
* [libboost_regex](https://github.com/boostorg)
* [libavcodec, libavformat, libavutil, libswresample](https://github.com/FFmpeg/FFmpeg)
* [libmujs](https://github.com/ccxvii/mujs)
#### Windows
Using [vcpkg](https://vcpkg.io) to install dependencies:
```ps
> vcpkg install fmt
> vcpkg install spdlog
> vcpkg install curl
> vcpkg install dpp
> vcpkg install boost
> vcpkg install ffmpeg
> vcpkg install mujs
```
Environment variable `REAL_VCPKG_ROOT` should be set to the root directory of `vcpkg`.
KontraBot can then be built using Visual Studio IDE.
#### Linux
The dependencies can be installed using package managers like apt, yum, etc.
They also can be built and installed from source. KontraBot can then be built like this:
```sh
$ mkdir build && cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release
$ make -j
```

## Installation
### 1. Filesystem
There are some files and directories that KontraBot needs in order to run. They can be generated with:
```sh
$ ./KontraBot -g
```

### 2. Configuration
KontraBot's `config.json` file needs to be configured before the bot can start. Configuration fields:

* `discord_bot_api_token`: The token used to connect to Discord. Can be obtained [here](https://discord.com/developers/docs/quick-start/getting-started).
* `proxy` - proxy server configuration:
  + `enabled`: Whether to use proxy when accessing YouTube servers or not.
  + `host`: Proxy server IP or domain.
  + `port`: Proxy server port.
  + `auth` - proxy server authentication configuration:
    + `required`: Whether proxy server requires authentication or not.
    + `user`: Proxy server authentication user.
    + `password`: Proxy server authentication password.

### 3. Slashcommands registration
KontraBot uses slashcommands. They have to be registered before Discord users can see them. 
```sh
$ ./KontraBot -r
```

### 4. Start
Everything is ready and the bot can start now. To start normally, no arguments have to be provided:
```sh
$ ./KontraBot
```
The bot will start initialization and after `Ready` message users can start sending requests.

### General help
KontraBot's help message can be called with:
```sh
$ ./KontraBot -h
```
