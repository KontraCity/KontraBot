# KontraBot
The simplest Discord music bot that can play videos and playlists from YouTube.
Supports multiple languages, notices video chapters and shows playing items in voice channel status.

## Dependencies
* [libfmt](https://github.com/fmtlib/fmt)
* [libcurl](https://github.com/curl/curl)
* [libdpp](https://github.com/brainboxdotcc/DPP)
* [libspdlog](https://github.com/gabime/spdlog)
* [libmujs](https://github.com/ccxvii/mujs)
* [libboost_regex](https://github.com/boostorg/regex)
* [libavcodec, libavformat, libavutil, libswresample](https://github.com/FFmpeg/FFmpeg)

## Build
You're going to have to install all the dependencies on your own. After that, the process is simple:
```sh
$ mkdir build && cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release
$ make -j
```

## Installation
### 1. Filesystem
There are some files and directories that KontraBot needs in order to run normally. They can be generated with:
```sh
$ ./KontraBot -g
```

### 2. Configuration
KontraBot's `config.json` file needs to be configured before the bot can start. Configuration fields:
1. `discord_bot_api_token`: The token used to connect to Discord. Can be obtained [here](https://discord.com/developers/docs/quick-start/getting-started).

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
