var jsonObj = {
    // the final bookmarks object to sync
    endpoint: "",
    pin: "",
    bookmarks: [ ]
};


document.addEventListener("click", (e) => {
    let options = browser.storage.sync.get();
    options.then((res) => {
        if (e.target.id == "ymarks-syncul") {
            // The user aimed to push.
            jsonObj.pin = parseInt(res.pin) || '';
            jsonObj.endpoint = res.endpoint || '';
            browser.runtime.sendMessage({
                func: "upload",
                json: jsonObj
            });
        }
        else if (e.target.id == "ymarks-syncdl") {
            // The user aimed to pull.
            jsonObj.pin = parseInt(res.pin) || '';
            jsonObj.endpoint = res.endpoint || '';
            browser.runtime.sendMessage({
                func: "download",
                json: jsonObj
            });
        }
        else if (e.target.id == "ymarks-restore") {
            // The user aimed to PANIC!
            jsonObj.pin = parseInt(res.pin) || '';
            jsonObj.endpoint = res.endpoint || '';
            browser.runtime.sendMessage({
                func: "restore",
                json: jsonObj
            });
        }
        else if (e.target.id == "ymarks-quit") {
            // The user aimed to kill the server.
            jsonObj.pin = parseInt(res.pin) || '';
            jsonObj.endpoint = res.endpoint || '';
            browser.runtime.sendMessage({
                func: "quit",
                json: jsonObj
            });
        }
    });
});
