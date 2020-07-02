function saveOptions(e) {
    chrome.storage.sync.set({
        endpoint: document.getElementById("endpoint").value,
        pin: document.getElementById("pin").value,
        auto_ul: document.getElementById("auto_ul").checked,
        auto_dl: document.getElementById("auto_dl").checked
    });
}

function restoreOptions() {
    chrome.storage.sync.get(
        {
            endpoint: '',
            pin: '',
            auto_ul: false,
            auto_dl: false
        },
        function(res) {
            document.getElementById("endpoint").value = res.endpoint || '';
            document.getElementById("pin").value = res.pin || '';
            document.getElementById("auto_ul").checked = res.auto_ul || false;
            document.getElementById("auto_dl").checked = res.auto_dl || false;
        });
}

document.addEventListener('DOMContentLoaded', restoreOptions);
document.getElementById("saveOptions").addEventListener("click", saveOptions);
