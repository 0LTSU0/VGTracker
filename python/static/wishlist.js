IGDB_PLATFORM_MAP = {
    "PS5": 167,
    "PS4": 48,
    "PSVR": 165,
    "PSVR2": 390,
    "PC": 6,
    "Nintendo Switch": 130,
    "Nintendo Switch 2": 508,
    "Xbox Series X|S": 169
}

const today = new Date();
let calYear  = today.getFullYear();
let calMonth = today.getMonth();

const MONTHS   = ["January","February","March","April","May","June",
                  "July","August","September","October","November","December"];
const DAY_HDRS = ["Mon","Tue","Wed","Thu","Fri","Sat","Sun"];
function prevMonth() { if (--calMonth < 0)  { calMonth = 11; calYear--; } drawCalendar(); }
function nextMonth() { if (++calMonth > 11) { calMonth = 0;  calYear++; } drawCalendar(); }

function renderGrid(byDate) {
    const daysInMonth = new Date(calYear, calMonth + 1, 0).getDate();
    const startOffset = (new Date(calYear, calMonth, 1).getDay() + 6) % 7; // Mon = 0

    let html = "";
    for (let i = 0; i < startOffset; i++)
        html += `<div class="calendar-day is-filler"></div>`;

    for (let d = 1; d <= daysInMonth; d++) {
        const ds      = `${calYear}-${String(calMonth+1).padStart(2,"0")}-${String(d).padStart(2,"0")}`;
        const isToday = d === today.getDate() && calMonth === today.getMonth() && calYear === today.getFullYear();
        const chips   = (byDate[ds] || []).map(g => {
            const lbl = g.name.length > 17 ? g.name.slice(0, 15) + "…" : g.name;
            const plats = (g.platforms || []).join(", ");
            return `<div class="release-chip not-wishlisted" title="${g.name} — ${plats} (${(g.igdb_hypes||0).toLocaleString()} hypes)">${lbl}</div>`;
        }).join("");
        html += `<div class="calendar-day${isToday ? " is-today" : ""}">
            <div class="calendar-day-num">${d}</div>${chips}</div>`;
    }

    const tail = (startOffset + daysInMonth) % 7;
    if (tail) for (let i = 0; i < 7 - tail; i++)
        html += `<div class="calendar-day is-filler"></div>`;

    document.getElementById("calendarGrid").innerHTML = html;
}

async function drawCalendar(platFilter, minIGDBWantList)
{
    document.getElementById("calendarTitle").textContent = `${MONTHS[calMonth]} ${calYear}`;
    document.getElementById("calendarHeaders").innerHTML =
        DAY_HDRS.map(d => `<div class="calendar-day-header">${d}</div>`).join("");

    // Draw empty grid immediately
    renderGrid({});

    // Build API URL
    let url = `/api/igdb/upcoming?year=${calYear}&month=${calMonth + 1}`;
    if (platFilter)      url += `&platform_id=${platFilter}`;
    if (minIGDBWantList) url += `&min_hypes=${minIGDBWantList}`;

    const res   = await fetch(url);
    const games = await res.json();

    const byDate = {};
    for (const g of games) {
        if (!g.release_date) continue;
        (byDate[g.release_date] = byDate[g.release_date] || []).push(g);
    }

    renderGrid(byDate);
}

function applyFilters()
{
    const platName = document.getElementById("filterPlatform").value
    const platId = IGDB_PLATFORM_MAP[platName] ?? null
    drawCalendar(platId, parseInt(document.getElementById("filterWantlist").value))
}

function buildPlatformFilter() {
    const sel = document.getElementById("filterPlatform");
    sel.innerHTML = `<option value="">All platforms</option>` +
        Object.keys(IGDB_PLATFORM_MAP).map(p =>
            `<option value="${p}">${p}</option>`
        ).join("");
}

//MARK: init
async function init() {
    buildPlatformFilter()
    const platName = document.getElementById("filterPlatform").value
    drawCalendar(IGDB_PLATFORM_MAP[platName] ?? null, parseInt(document.getElementById("filterWantlist").value))
}
document.addEventListener("DOMContentLoaded", init)