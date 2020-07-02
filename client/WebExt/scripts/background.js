_jsonObj = {};

let dictOldIDsToNewIDs = { "-1": "-1" };


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
        folderID: bookmarkItem.parentId || -1,
        ID: bookmarkItem.id,
        type: bookmarkItem.type,
        title: bookmarkItem.title,
        url: bookmarkItem.url,
        position: bookmarkItem.index
    };

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

    if (bmid == "root________") {
        // Let's skip the root node.
        addAllBookmarks(jsonMarks, ++index);
        return;
    }

    // If the parentId ends with "__", chances are that this is a system folder.
    // Do not touch it.
    let newParentId = (typeof bmparentId !== 'undefined' && bmparentId.substr(bmparentId.length - 2) == "__") ? bmparentId : dictOldIDsToNewIDs[bmparentId];

    // Create the bookmark:
    browser.bookmarks.create(
        (bmtype == "separator" ?
         {
             index: bmindex,
             parentId: newParentId,
             type: bmtype
         } :
         (bmtype == "folder" ?
          {
              index: bmindex,
              parentId: newParentId,
              title: bmtitle,
              type: bmtype
          } :
          /* bmtype == bookmark */
          {
              index: bmindex,
              parentId: newParentId,
              title: bmtitle,
              type: bmtype,
              url: bmurl
          }
         )
        )
    ).then(function(node) {
        // There probably is a new ID set for this bookmark.
        // This will affect all child nodes.
        //   Old:  bmid
        //   New:  node.id

        let newID = bmid.substr(bmid.length - 2) == "__" ? bmid : node.id;
        dictOldIDsToNewIDs[bmid] = newID;

        // recursive call.
        if (typeof jsonMarks[index+1] !== 'undefined') {
            addAllBookmarks(jsonMarks, ++index);
            return;
        }
        else {
            // Done, I guess.
            yalert("Successfully synchronized " + index + " bookmarks and folders. :-)");
        }
    }, function(err) {
        // Something did not quite run through.
        yalert("Skipping the bookmark " + bmtitle + " (" + bmurl + ") due to invalid data.");
        
        // TODO: We ignore this state now - but there shouldn't be invalid data.
        // As this happened first with the bookmarks toolbar folder: could it be
        // that Firefox has certain default folders which can't be "synced"?
        addAllBookmarks(jsonMarks, ++index);
    });
}


function failureParsingBookmarksTree(error) {
    yalert("Failed to parse your bookmarks tree: " + error);
    document.querySelector("#ymarks-status").html("Failed to parse your bookmarks tree: " + error);
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
        yalert("Downloading failed. :-( Reason: " + data.message);
    }
    else {
        let jsonMarks = JSON.parse(data.message);
        if (typeof jsonMarks[0] !== 'undefined') {
            // remove all local bookmarks
            browser.bookmarks.getTree().then(function(tree) {
                if (tree[0].id) {
                    // remove all local bookmarks
                    browser.bookmarks.getChildren("toolbar_____").then(
                        function(results) {
                            for (i = 0; i < results.length; i++) {
                                browser.bookmarks.removeTree(results[i].id);
                            }
                            browser.bookmarks.getChildren("menu________").then(
                                function(results) {
                                    for (i = 0; i < results.length; i++) {
                                        browser.bookmarks.removeTree(results[i].id);
                                    }
                                    browser.bookmarks.getChildren("unfiled_____").then(
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
                                    yalert("Could not remove your local Bookmarks Menu bookmarks (downloading new ones anyway): " + err);
                                    addAllBookmarks(jsonMarks, 0)
                                });
                        },
                        function(err) {
                            yalert("Could not remove your local Bookmarks Toolbar bookmarks (downloading new ones anyway): " + err);
                            addAllBookmarks(jsonMarks, 0)
                        });
                }
                else {
                    // The root node has no id? Hmm.
                    yalert("Could not remove your local bookmarks: There are none.");
                    addAllBookmarks(jsonMarks, 0);
                }
            }, function(err) {
                yalert("Could not remove your local bookmarks, we'll just repopulate your database: " + err);
                addAllBookmarks(jsonMarks, 0);
            });
        }
        else {
            // ?!
            yalert("There are no bookmarks stored on your server yet. :-)");
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
