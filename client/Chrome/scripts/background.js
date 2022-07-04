/**
 * ymarks :: a self-hosted single-user bookmark sync service.
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.1 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * See the file COPYING in this distribution for details.
 * A copy of the CDDL is also available via the Internet at
 * https://spdx.org/licenses/CDDL-1.1.html
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the contents of the COPYING file from this
 * distribution.
 */

_jsonObj = {};

let dictOldIDsToNewIDs = { "-1": "-1" };

// Some Chrome<->Firefox translations
let cToF = {"0": "root________", "1": "toolbar_____", "2": "menu________"};
let fToC = {"root________": "0",  "toolbar_____" : "1", "menu________": "2"};
let firefoxRootId = "root________";
let firefoxOtherBookmarksId = "unfiled_____";
let firefoxOtherBookmarks = "Other Bookmarks"; // TODO: Maybe translate this later...
let chromeFirefoxOtherBookmarks = "Other Bookmarks (Firefox)"; // TODO: Maybe translate this later...
let chromeFirefoxOtherBookmarksId = "";

function chromeToFirefox(folderId) {
    return cToF[folderId] || folderId;
}

function firefoxToChrome(folderId) {
    return fToC[folderId] || dictOldIDsToNewIDs[folderId] || folderId;
}

function isSystemId(bmid) {
    return typeof bmid !== 'undefined' && bmid.substr(bmid.length - 2) == "__";
}

function isRoot(bmid) {
    return bmid == firefoxRootId;
}

function isFirefoxOtherBookmarks(bmid) {
    return bmid == firefoxOtherBookmarksId;
}


function yalert(msg) {
    // Browser notifications instead of alert()s. Yay.
    browser.notifications.create({
        "type": "basic",
        "iconUrl": browser.extension.getURL("icons/ymarks-32.png"),
        "title": "ymarks",
        "message": msg
    });
}


function check_config() {
    // Displays a warning about missing configuration if applicable.
    // Returns true if everything's OK.
    if (_jsonObj.pin == '' || _jsonObj.endpoint == '') {
        yalert("You forgot to set up the sync service adequately. Please check the ymarks extension options, then try again.");
        return false;
    }

    return true;
}

function fetchLocalBookmarks(bookmarkItem) {
    let thisBookmarkJSON = {
        // build the bookmark
        // elements:
        //   - folderID: the ID of the parent folder
        //   - ID:       the internal ID of the bookmark
        //   - type:     the bookmark's type
        //   - title:    the title of the bookmark
        //   - url:      where the bookmark leads
        //   - position: for remembering the order
        //
        // TODO: handle tags once the WebExt API supports that.
        folderID: chromeToFirefox(bookmarkItem.parentId) || -1,
        ID: chromeToFirefox(bookmarkItem.id),
        type: bookmarkItem.type,
        title: bookmarkItem.title,
        url: bookmarkItem.url,
        position: bookmarkItem.index
    };

    if (bookmarkItem.children !== 'uderfined' && bookmarkItem.title == chromeFirefoxOtherBookmarks) {
        // Map to the 'unfiled_____'
        chromeFirefoxOtherBookmarksId = bookmarkItem.id;
        thisBookmarkJSON.folderID = firefoxRootId;
        thisBookmarkJSON.ID = firefoxOtherBookmarksId;
        thisBookmarkJSON.title = firefoxOtherBookmarks;
    }

    if (bookmarkItem.parentId == chromeFirefoxOtherBookmarksId) {
        // Map to the 'unfiled_____'
        thisBookmarkJSON.folderID = firefoxOtherBookmarksId;
    }
    // there is no sane reason to add complex nesting while
    // preparing the list of bookmarks for uploading. the
    // server can perfectly processt bhem anyway.
    _jsonObj.bookmarks.push(thisBookmarkJSON);

    if (bookmarkItem.children) {
        for (child of bookmarkItem.children) {
            fetchLocalBookmarks(child);
        }
    }
}


function parseLocalBookmarksTree(bookmarkItems) {
    fetchLocalBookmarks(bookmarkItems[0]);
    yalert("ymarks collected your bookmarks. We will try to upload them now. Processing can take a short while (depending on your collection), please wait! 😊");

    // We probably have a list of bookmarks now. Try to push them to our server.
    fetch(_jsonObj.endpoint + "/upload", {
        method: 'post',
        body: JSON.stringify(_jsonObj)
    }).then(function(response) {
        return response.json();
    }).then(function(data) {
        if (!data.success) {
            yalert("Uploading failed. :-( Reason: " + data.message);
        }
        else {
            yalert(data.message);
        }
    }).catch(function(err) {
        // Whoops...?
        yalert("Failed to send a request to your endpoint: " + err.message);
    });
}

function addAllBookmarks(jsonMarks, index) {
    // add all bookmarks to our now empty tree
    let bmid = jsonMarks[index].ID; // this will change while inserting
    let bmparentId = jsonMarks[index].folderID; // this has to be adjusted accordingly
    let bmindex = jsonMarks[index].position;
    let bmtitle = jsonMarks[index].title;
    let bmtype = jsonMarks[index].type;
    let bmurl = jsonMarks[index].url;

    if (isFirefoxOtherBookmarks(bmid)) {
        // Special treatment for FireFox "Other Bookmarks" Put it under "2"
        bmparentId = "menu________";
        bmtitle = chromeFirefoxOtherBookmarks;
    }
    else if (isSystemId(bmid) || isRoot(bmparentId)) {
        // Skip system folders and direct children of the root folder.
        addAllBookmarks(jsonMarks, ++index);
        return;
    }

    let newParentId = firefoxToChrome(bmparentId);

    // Create the bookmark:
    if (bmtype != "separator") {
        browser.bookmarks.create(
             (bmtype == "folder" ?
              {
                  index: bmindex,
                  parentId: newParentId,
                  title: bmtitle
              } :
              /* bmtype == bookmark */
              {
                  index: bmindex,
                  parentId: newParentId,
                  title: bmtitle,
                  url: bmurl
              }
            )
        ).then(function(node) {
            // There probably is a new ID set for this bookmark.
            // This will affect all child nodes.
            //   Old:  bmid
            //   New:  node.id

           if (bmtype == "folder") {
               dictOldIDsToNewIDs[bmid] = node.id;
           }

            // recursive call.
            if (typeof jsonMarks[index+1] !== 'undefined') {
                addAllBookmarks(jsonMarks, ++index);
                return;
            }
            else {
                // Done, I guess.
                yalert("Successfully synchronized " + index + " bookmarks and folders. 😊");
            }
        }, function(err) {
            // Something did not quite run through.
            yalert("Skipping the bookmark " + bmtitle + " (" + bmurl + ") due to invalid data.");
        })
    } else {
        // skip + continue
        addAllBookmarks(jsonMarks, ++index);
        return;
    }
}


function failureParsingBookmarksTree(error) {
    document.querySelector("#ymarks-status").innerHTML = "Failed to parse your bookmarks tree: " + error;
}


function sync_ul() {
    // try to upload local bookmarks:
    if (check_config()) {
        let gettingTree = browser.bookmarks.getTree();
        gettingTree.then(parseLocalBookmarksTree, failureParsingBookmarksTree);
    }
}

function process_downloaded_bookmarks(data) {
    // removes all existing bookmarks and adds new ones if <data> contains any.
    if (!data.success) {
        yalert("Downloading failed. Reason: " + data.message);
    }
    else {
        let jsonMarks = JSON.parse(data.message);
        if (typeof jsonMarks[0] !== 'undefined') {
            // remove all local bookmarks
            browser.bookmarks.getChildren("1").then(
                function(results) {
                    for (i = 0; i < results.length; i++) {
                        browser.bookmarks.removeTree(results[i].id);
                    }
                    browser.bookmarks.getChildren("2").then(
                    function(results) {
                        for (i = 0; i < results.length; i++) {
                            browser.bookmarks.removeTree(results[i].id);
                        }
                        addAllBookmarks(jsonMarks, 0)
                    },
                    function(err) {
                        yalert("Could not remove your local Other Bookmarks (downloading new ones anyway): " + err);
                        addAllBookmarks(jsonMarks, 0)
                    });
                },
                function(err) {
                    yalert("Could not remove your local Bookmarks Bar bookmarks (downloading new ones anyway): " + err);
                    addAllBookmarks(jsonMarks, 0)
                }
            );
        }
        else {
            // ?!
            yalert("There are no bookmarks stored on your server yet. 😊");
        }
    }
}


function sync_dl() {
    // try to download remote bookmarks:
    if (check_config()) {
        fetch(_jsonObj.endpoint + "/download", {
            method: 'post',
            body: JSON.stringify(_jsonObj)
        }).then(function(response) {
            return response.json();
        }).then(function(data) {
            process_downloaded_bookmarks(data);
        }).catch(function(err) {
            // Whoops...?
            yalert("Failed to send a request to your endpoint: " + err.message);
        });
    }
}


function sync_restore() {
    // try to download the previous version (if applicable):
    if (check_config()) {
        fetch(_jsonObj.endpoint + "/restore", {
            method: 'post',
            body: JSON.stringify(_jsonObj)
        }).then(function(response) {
            return response.json();
        }).then(function(data) {
            process_downloaded_bookmarks(data);
        }).catch(function(err) {
            // Whoops...?
            yalert("Failed to send a request to your endpoint: " + err.message);
        });
    }
}


function quit_server() {
    // try to quit the server:
    if (check_config()) {
        fetch(_jsonObj.endpoint + "/quit", {
            method: 'post',
            body: JSON.stringify(_jsonObj)
        }).then(
            function(response) {},
            function(error) {
                // If there was an error, the server is probably unreachable now.
                yalert("The endpoint " + _jsonObj.endpoint + " seems to have shut down now. Feel free to restart it whenever you need it.");
            });
    }
}


// -----------------------------------------------------
// Message handler: This will basically do all the work.

function handleMessage(request, sender, sendResponse) {
    if (request.json) {
        _jsonObj = request.json;
    }

    if (request.func == "upload") {
        sync_ul();
    }
    else if (request.func == "download") {
        sync_dl();
    }
    else if (request.func == "restore") {
        sync_restore();
    }
    else if (request.func == "quit") {
        quit_server();
    }
}


// -----------------------------------------------------
// Automatic bookmark upload:

function handleBookmarkChange() {
    let options = browser.storage.sync.get();
    options.then((res) => {
        if (res.auto_ul && check_config()) {
            let gettingTree = browser.bookmarks.getTree();
            gettingTree.then(parseLocalBookmarksTree, failureParsingBookmarksTree);
        }
    });
}


// -----------------------------------------------------
// Automatic bookmark download:

function handleBrowserStart() {
    let options = browser.storage.sync.get();
    options.then((res) => {
        if (res.auto_dl) {
            sync_dl();
        }
    });
}


// -----------------------------------------------------
// Install the various listeners.

browser.runtime.onMessage.addListener(handleMessage);
browser.runtime.onStartup.addListener(handleBrowserStart);
browser.bookmarks.onCreated.addListener(handleBookmarkChange);
browser.bookmarks.onChanged.addListener(handleBookmarkChange);
