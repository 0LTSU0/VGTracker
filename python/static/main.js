
let _platforms = {}
let _pendingTempCoverKey = null

async function getPlatforms(){
    const res = await fetch("/api/platforms", {})
    const platforms = await res.json()
    document.getElementById("num_platforms").textContent = String(platforms.length)
    const container = document.getElementById("platformOptions")
    container.innerHTML = ""
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
    if (!platform_id) {
        // Only update when were fetching all
        document.getElementById("num_games").textContent = String(entries.length)
    }
    
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
                <figure class="image is-3by4 is-64x64">
                    <img src="/covers/${e.id}.png" id="thumbnail_${e.id}" style="height: 64px; width: 48px" />
                </figure>
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
        res = await fetch("/api/entries",{
            method:"PUT",
            headers:{"Content-Type":"application/json"},
            body:JSON.stringify(eObj)
        })
    } else {
        console.log("Creting new entry", eObj)
        res = await fetch("/api/entries",{
            method:"POST",
            headers:{"Content-Type":"application/json"},
            body:JSON.stringify(eObj)
        })
    }
    if (res.status == 200) {
        const saved = await res.json()
        const savedId = saved.id
        if (_pendingTempCoverKey && savedId) {
            await fetch(`/api/entries/${savedId}/cover/from_temp?key=${_pendingTempCoverKey}`, { method: "POST" })
            _pendingTempCoverKey = null
        }
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
    document.getElementById("igdbSuggestions").innerHTML = "" // clear old igdb search result
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

    if (entry.id) {
        document.getElementById("details_modal_delete_entry").removeAttribute("disabled")
        const img = document.getElementById("details_modal_cover_img")
        img.src = `/covers/${entry.id}.png`
    }
}

function newEntry() {
    document.getElementById("igdbSuggestions").innerHTML = ""  // clear old igdb search result
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
    document.getElementById("details_modal_entry_id").value = null

    document.getElementById("details_modal_delete_entry").setAttribute("disabled", true)
    const img = document.getElementById("details_modal_cover_img")
    img.src = `/covers/placeholder.png`
}

function updateDetailModalContentIGDB(item) {
    /* 
    python response filled fields are
    return {
        "developer": developers,
        "publisher": publishers,
        "series": series,
        "genres": genres,
        "release_year": rel_year,
        "name": full_game_entry.get("name")
    }
    */
    document.getElementById("igdbSuggestions").innerHTML = ""  // clear old igdb search result
    document.getElementById("details_modal_title_header").innerText = item.name
    document.getElementById("details_modal_title").value = item.name
    document.getElementById("details_modal_developer").value = item.developer
    document.getElementById("details_modal_publisher").value = item.publisher
    document.getElementById("details_modal_series").value = item.series
    document.getElementById("details_modal_genres").value = item.genres
    document.getElementById("details_modal_release_year").value = item.release_year

    _pendingTempCoverKey = item.cover_temp_key || null
    if (_pendingTempCoverKey) {
        document.getElementById("details_modal_cover_img").src = `/covers/temp_${_pendingTempCoverKey}.png?t=${Date.now()}`
    }
}

async function deleteEntry() {
    let id = document.getElementById("details_modal_entry_id").value
    if (!id) {
        console.log("no value in details_modal_entry_id. Cannot delete")
        return
    }
    if (!confirm("Delete this entry?")) return

    await fetch(`/api/entries/${id}`, {
        method: "DELETE"
    })

    closeDetailsModal()
    loadEntries()
}

function openDetailsModal(entry) {
    console.log("Open details modal for", entry)
    updateDetailModalContent(entry)
    document.getElementById("details_modal").classList.add("is-active")
}

function closeDetailsModal() {
    document.getElementById("details_modal").classList.remove("is-active")
    document.getElementById("igdbSuggestions").style.display = "none"
    _pendingTempCoverKey = null
}

async function detailsModalSaveChanges() {
    await postEntryObj()
}

function logout() {
    window.location = "/logout"
}

function openPlatformModal() {
    document.getElementById("platformModal").classList.add("is-active")
}

function closePlatformModal() {
    document.getElementById("platformModal").classList.remove("is-active")
}

async function savePlatform() {
    const name = document.getElementById("platform_modal_name").value
    const supports_platinum =
        document.getElementById("platform_modal_supports_platinum").checked

    let res = await fetch("/api/platforms",{
        method:"POST",
        headers:{"Content-Type":"application/json"},
        body:JSON.stringify({"name": name, "supports_platinum": supports_platinum})
    })
    if (res.status == 200) {
        let js = await res.json()
        loadEntries(js.id)
        closePlatformModal()
        document.getElementById("dropdownLabel").innerText = js.name
        getPlatforms()
    }
}


//MARK: IGDB INTEGRATION
async function searchIGDB() {
    const title = document.getElementById("details_modal_title").value
    if (!title) return

    document.getElementById("details_modal_title_control").classList.add("is-loading")

    const res = await fetch(`/api/igdb/search?name=${encodeURIComponent(title)}`)
    const results = await res.json()

    const box = document.getElementById("igdbSuggestions")
    box.innerHTML = ""
    box.style.display = "block"
    results.forEach(game => {

        const year = game.first_release_date
            ? new Date(game.first_release_date * 1000).getFullYear()
            : "?"

        const item = document.createElement("div")
        item.className = "dropdown-item"
        item.style.cursor = "pointer"

        item.textContent = `${game.name} (${year})`

        item.onclick = () => selectIGDBGame(game)

        box.appendChild(item)
    })
    document.getElementById("details_modal_title_control").classList.remove("is-loading")
}


async function selectIGDBGame(game) {
    console.log("selectIGDBgame", game)
    makePageLoading()
    document.getElementById("igdbSuggestions").style.display = "none"
    const res = await fetch(`/api/igdb/getgame_v2?id=${game.id}`)
    if (res.status == 200) {
        updateDetailModalContentIGDB(await res.json())
    }
    hidePageLoading()
}


function makePageLoading() {
    document.getElementById("loadingOverlay").style.display = "flex"
}
function hidePageLoading() {
    document.getElementById("loadingOverlay").style.display = "none"
}


//MARK: COVER UPLOAD
function openCoverUpload() {
    const entryId = document.getElementById("details_modal_entry_id").value
    if (!entryId) {
        alert("Save the entry before uploading a cover.")
        return
    }
    const input = document.createElement("input")
    input.type = "file"
    input.accept = "image/*"
    input.onchange = async () => {
        const file = input.files[0]
        if (!file) return
        const form = new FormData()
        form.append("file", file)
        const res = await fetch(`/api/entries/${entryId}/cover`, {
            method: "POST",
            body: form
        })
        if (res.ok) {
            const img = document.getElementById("details_modal_cover_img")
            img.src = `/covers/${entryId}.png?t=${Date.now()}`
        } else {
            alert("Cover upload failed.")
        }
    }
    input.click()
}


//MARK: init
async function init() {
    await getPlatforms() // we need to make sure this finished before loading entries so they have platform map available
    await loadEntries()
}
document.addEventListener("DOMContentLoaded", init)
