
let _platforms = {}

async function getPlatforms(){
    const res = await fetch("/api/platforms", {})
    const platforms = await res.json()
    document.getElementById("num_platforms").textContent = String(platforms.length)
    const container = document.getElementById("platformOptions")
    const modal_container = document.getElementById("details_modal_platform")
    platforms.forEach(p => {
        const a = document.createElement("a")
        a.className = "dropdown-item"
        a.innerText = p.name
        a.onclick = () => selectPlatform(p)
        container.appendChild(a)

        const option = document.createElement("option")
        option.value = p.id
        option.textContent = p.name
        modal_container.appendChild(option)
    })
    platforms.forEach(p => {
        _platforms[p.id] =  {"name": p.name, "supports_platinum": p.supports_platinum}
    })
    const a = document.createElement("a")
    a.className = "dropdown-item"
    a.innerText = "All"
    a.onclick = () => selectPlatform()
    container.appendChild(a)
}

async function loadEntries(platform_id){
    url = "/api/entries"
    if (platform_id) {
        url = url + `?platform_id=${platform_id}`
    }
    const res = await fetch(url, {})
    const entries = await res.json()
    document.getElementById("num_games").textContent = String(entries.length)

    const table = document.getElementById("entriesTable")
    table.innerHTML = ""
    entries.forEach(e => {
        const platformname = _platforms[e.platform_id].name || String(webkitURL.platform_id)
        const row = document.createElement("tr")
        let plat_col = "-"
        if (!_platforms[e.platform_id].supports_platinum) {
            plat_col = "N/A"
        } else if (e.platinumed) {
            plat_col = "🏆"
        }
        const button_id = "details_button_details_" + e.id
        row.innerHTML = `
            <td>
                N/A
            </td>
            <td>${e.title}</td>
            <td>${e.status || ""}</td>
            <td>${plat_col}</td>
            <td>${platformname}</td>
            <td>
                <button class="button is-small" id="${button_id}">Details</button>
            </td>
        `
        table.appendChild(row)
        document.getElementById(button_id).onclick = () => openDetailsModal(e)
    })
}

async function createEntry(){

    await fetch("/entries",{
        method:"POST",
        headers:{
            "Content-Type":"application/json",
            "Authorization":"Bearer "+token
        },
        body:JSON.stringify({
            title:document.getElementById("title").value,
            content:document.getElementById("content").value
        })
    })

    loadEntries()
}

function toggleDropdown() {
    const dropdown = document.getElementById("platformDropdown")
    dropdown.classList.toggle("is-active")
}

function selectPlatform(platform){
    if (!platform) { // No platform -> show all
        document.getElementById("dropdownLabel").innerText = "All"
        document.getElementById("platformDropdown")
            .classList.remove("is-active")
        console.log("Selected platform: ALL")
        loadEntries()
    } else {
        document.getElementById("dropdownLabel").innerText = platform.name
        document.getElementById("platformDropdown")
            .classList.remove("is-active")
        console.log("Selected platform:", platform.id)
        loadEntries(platform.id)
    }
}

async function postEntryObj(){
    eObj = {
        "status": document.getElementById("details_modal_status").value,
        "rating": parseFloat(document.getElementById("details_modal_rating").value),
        "publisher": document.getElementById("details_modal_publisher").value,
        "series": document.getElementById("details_modal_series").value,
        "platinumed": document.getElementById("details_modal_platinum").value === "Yes",
        "user_id": parseInt(localStorage.getItem("user_id")),
        "title": document.getElementById("details_modal_title").value,
        "notes": document.getElementById("details_modal_notes").value,
        "release_year": parseInt(document.getElementById("details_modal_release_year").value),
        "developer": document.getElementById("details_modal_developer").value,
        "genres": document.getElementById("details_modal_genres").value,
        "cover_path": null,
        "price": parseFloat(document.getElementById("details_modal_price").value),
        "platform_id": parseInt(document.getElementById("details_modal_platform").value)
    }
    hidden_id_field = document.getElementById("details_modal_entry_id")
    let res = null
    if (hidden_id_field.value) {
        eObj["id"] = hidden_id_field.value
        console.log("Doing update since hidden id field has value", eObj)
    } else {
        console.log("Creting new entry", eObj)
        res = await fetch("/api/entries",{
            method:"POST",
            headers:{"Content-Type":"application/json"},
            body:JSON.stringify(eObj)
        })
    }
    if (res.status == 200) {
        loadEntries(eObj.platform_id)
        closeDetailsModal()
    } else {
        let error = await res.json()
        console.error("Entry obj post failed", error)
    }
}

function updateTitle(val) {
    document.getElementById("details_modal_title_header").innerText = val
}

function updateDetailModalContent(entry) {
    console.log("Updating modal content for", entry)
    document.getElementById("details_modal_title_header").innerText = entry.title
    document.getElementById("details_modal_title").value = entry.title
    document.getElementById("details_modal_series").value = entry.series
    document.getElementById("details_modal_platform").value = entry.platform_id
    document.getElementById("details_modal_status").value = entry.status
    details_modal_platinum = document.getElementById("details_modal_platinum")
    if (_platforms[entry.platform_id].supports_platinum) {
        details_modal_platinum.disabled = false
        details_modal_platinum.title = ""
        details_modal_platinum.value = entry.platinumed ? "Yes" : "No"
    } else {
        details_modal_platinum.disabled = true
        details_modal_platinum.title = "Platinums not supported on this platform"
        details_modal_platinum.value = "No"
    }
    document.getElementById("details_modal_price").value = entry.price
    document.getElementById("details_modal_developer").value = entry.developer
    document.getElementById("details_modal_release_year").value = entry.release_year
    document.getElementById("details_modal_rating").value = entry.rating
    document.getElementById("details_modal_publisher").value = entry.publisher
    document.getElementById("details_modal_genres").value = entry.genres
    document.getElementById("details_modal_notes").value = entry.notes
    document.getElementById("details_modal_entry_id").value = entry.id
}

function newEntry() {
    document.getElementById("details_modal_title_header").innerText = "New entry"
    document.getElementById("details_modal_title").value = null
    document.getElementById("details_modal_series").value = null
    document.getElementById("details_modal_platform").value = null
    document.getElementById("details_modal_status").value = null
    details_modal_platinum = document.getElementById("details_modal_platinum")
    details_modal_platinum.disabled = false
    details_modal_platinum.title = ""
    details_modal_platinum.value = "No"
    document.getElementById("details_modal_price").value = null
    document.getElementById("details_modal_developer").value = null
    document.getElementById("details_modal_release_year").value = null
    document.getElementById("details_modal_rating").value = null
    document.getElementById("details_modal_publisher").value = null
    document.getElementById("details_modal_genres").value = null
    document.getElementById("details_modal_notes").value = null
    document.getElementById("details_modal").classList.add("is-active")
}

function openDetailsModal(entry) {
    console.log("Open details modal for", entry)
    updateDetailModalContent(entry)
    document.getElementById("details_modal").classList.add("is-active")
}

function closeDetailsModal() {
    document.getElementById("details_modal").classList.remove("is-active")
}

async function detailsModalSaveChanges() {
    await postEntryObj()
}

async function init() {
    await getPlatforms() // we need to make sure this finished before loading entries so they have platform map available
    await loadEntries()
}
document.addEventListener("DOMContentLoaded", init)
