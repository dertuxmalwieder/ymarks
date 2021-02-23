# ymarks

A self-hosted solution to keep your browser's bookmarks synchronized without limiting yourself to one provider.

## Features

* Multi-browser support: If your browser speaks WebExtensions, you're in!
* KISS: Download the latest database or upload your current bookmarks. No "incremental" blah blah with extensive calculations etc.
* Open Source: Trust nobody (including me)!
* Backups: `ymarks` will clone your latest two databases in case you fucked it up.

## Requirements for the client part

Just grab the extension for your preferred WebExtension-capable browser from your browser's add-on portal, usually linked directly on [ymarks.org](https://www.ymarks.org).

Actually working support for browsers other than Firefox and Chrome is under way. :-)

## Usage

You'll need to set your server (by default, `http://localhost:8888`) and PIN (by default, `12345`) in your extension settings. If it worked, you can easily control your `ymarks` instance over the pop-up menu.

## TODO

* Support synchronization of bookmark tags. There does not seem to be a WebExtensions API for that yet.
