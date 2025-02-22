async function loadSerial() {
    try {
        let response = await fetch('/serial');
        if (!response.ok) throw new Error("Failed to load serial");
        let data = await response.text();
        document.getElementById('serial').innerHTML = data || "N/A";
    } catch (error) {
        console.error("Error loading serial:", error);
        showPopup("⚠️ The device can`t load the serial!");
    }
}

function showPopup(message) {
    let popup = document.createElement("div");
    popup.className = "popup";
    popup.textContent = message;
    document.body.appendChild(popup);

    setTimeout(() => {
        popup.style.opacity = "0";
        setTimeout(() => popup.remove(), 1000);
    }, 3000);
}
