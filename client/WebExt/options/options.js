function saveOptions(e) {
    browser.storage.sync.set({
        endpoint: document.querySelector("#endpoint").value,
        pin: document.querySelector("#pin").value,
        auto_ul: document.querySelector("#auto_ul").checked,
        auto_dl: document.querySelector("#auto_dl").checked
    });
    e.preventDefault();
}

function restoreOptions() {
    let options = browser.storage.sync.get();
    options.then((res) => {
        document.querySelector("#endpoint").value = res.endpoint || '';
        document.querySelector("#pin").value = res.pin || '';
        document.querySelector("#auto_ul").checked = res.auto_ul || false;
        document.querySelector("#auto_dl").checked = res.auto_dl || false;
    }, function (err) {
        document.querySelector("#endpoint").value = 'http://127.0.0.1:8888';
        document.querySelector("#pin").value = '12345';
    });
}

document.addEventListener('DOMContentLoaded', restoreOptions);
document.querySelector("#saveOptions").addEventListener("click", saveOptions);
