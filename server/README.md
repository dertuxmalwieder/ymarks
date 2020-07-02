# ymarks

A self-hosted solution to keep your browser's bookmarks synchronized without limiting yourself to one provider.

## Features

* Multi-browser support: If your browser speaks WebExtensions, you're in!
* KISS: Download the latest database or upload your current bookmarks. No "incremental" blah blah with extensive calculations etc.
* Open Source: Trust nobody (including me)!
* Backups: `ymarks` will clone your latest two databases in case you fucked it up.

## Requirements

### Server

You'll need a decent C compiler and the `sqlite3` libraries.

    cc -I . -I /usr/local/include -o ymarks 3rdparty/cJSON.c src/backup.c src/main.c -lsqlite3

Your exact compiler command could vary, depending on your library and include paths.

See the `build.cmd` file for a clue about how to compile with Clang on Windows.

#### Preprocessor definitions

* Define `SILENT` if you want the server to shut the fuck up while running, otherwise `ymarks` will log incoming requests to `stdout`.
* Define `NO_BACKUP` if you do not want `ymarks` to automatically create backup files.
* *Windows-only:*
 * Define `KEEP_VISIBLE` if you do not want to hide the server window after the start. (By default, it'll disappear after a successful start.)

### Client

Just grab the extension for your preferred WebExtension-capable browser from [ymarks.org](https://www.ymarks.org).

## Usage

### Set up the server

1. Run the executable. It will run on port 8888 by default. If you'd prefer to use a different port, please change the `SERVERPORT` integer in `constants.h`, recompile and try it again.
2. *(Optional)* Place a file named `PIN.txt` in the same folder as the compiled binary is in order to change your PIN from "12345" to something else. It will have to be a number though.

### Set up the browser extension(s)

Right now, we support Firefox and Chrome (less so, Vivaldi). Grab the extension as usual. :-)

## TODO

* Support synchronization of bookmark tags. There does not seem to be a WebExtensions API for that yet.

## Licenses

The server part of `ymarks` comes with a number of third-party libraries (see `/server/3rdparty`), licensed under the terms of the BSD resp. MIT licenses. Everything else in this repository is original work, licensed under the terms of the *WTFPL* license. Please refer to the `server/COPYING` file for details. Feel free to do whatever you want with it.
