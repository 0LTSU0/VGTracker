
import httpx
import os, json, time
from datetime import datetime
from fastapi import APIRouter

router = APIRouter()

IGDB_TOKEN = None
IGDB_HEADER = {"Client-ID": None, "Authorization": None}

async def do_request(url, query, is_retry=False, return_only_first=False):
    print("do_request called with", url, query)
    async with httpx.AsyncClient() as client:
        res = await client.post(
            url,
            headers=IGDB_HEADER,
            data=query
        )
        if res.status_code == 429 and not is_retry:
            print("igdb returned 'too many requests', retrying after two seconds")
            time.sleep(2)
            do_request(url, query, is_retry=True)
        elif res.status_code == 401 and not is_retry:
            print("igdb do_request returned unauthorized, trying to refresh token")
            await authenticate()
            do_request(url, query, is_retry=True)
        elif is_retry and res.status_code != 200:
            print("TODO error handling for do_request status code != 200")
    
    if not return_only_first:
        return res.json()
    else:
        js = res.json()
        return js[0]


async def authenticate():
    print("authenticating with igdb")
    global IGDB_TOKEN, IGDB_HEADER
    client_id = os.getenv("igdb_client_id")
    client_secret = os.getenv("igdb_client_secret")
    if not client_id or not client_secret:
        try:
            with open("igdb.json", "r") as f:
                j = json.load(f)
            client_id = j["client_id"]
            client_secret = j["client_secret"]
        except Exception as e:
            print("igdb authentication failed. Cannot find client id and/or secret from env or igdb.json", e)
            return
    async with httpx.AsyncClient() as client:
        url = f"https://id.twitch.tv/oauth2/token?client_id={client_id}&client_secret={client_secret}&grant_type=client_credentials"
        res = await client.post(url)
        if not res.status_code == 200:
            print("IGDB authentication failed with", res.status_code)
            return
        res_json = res.json()
        IGDB_TOKEN = res_json.get("access_token")
        IGDB_HEADER["Client-ID"] = client_id
        IGDB_HEADER["Authorization"] = f"Bearer {IGDB_TOKEN}"
        print("IGDB authentication successfull")


@router.get("/igdb/search")
async def igdb_search(name: str):
    global IGDB_TOKEN
    if not IGDB_TOKEN:
        await authenticate()

    query = f'''
    search "{name}";
    fields *;
    limit 25;
    '''

    async with httpx.AsyncClient() as client:
        res = await client.post(
            "https://api.igdb.com/v4/games",
            headers=IGDB_HEADER,
            data=query
        )
        if res.status_code == 401:
            print("igdb returned unauthorixed during query, setting IGDB_TOKEN to none")
            IGDB_TOKEN = None

    return res.json()


@router.get("/igdb/getgame") # get game in format our app uses internally
async def igdb_getgame(id: int):
    full_game_entry = await do_request("https://api.igdb.com/v4/games", f"fields *; where id = {id};", return_only_first=True) # there should only ever be one entry since were requesting by id

    series = "N/A"
    if full_game_entry.get("collections"):
        series_names = []
        for ser in full_game_entry.get("collections"):
            s = await do_request("https://api.igdb.com/v4/collections", f"fields name; where id = {ser};", return_only_first=True)
            if s.get("name"):
                series_names.append(s.get("name"))
        if series_names:
            series = ", ".join(series_names)

    developers, publishers = "N/A", "N/A"
    if full_game_entry.get("involved_companies"):
        developer_names, publisher_names = [], []
        for ic in full_game_entry.get("involved_companies"):
            involved_company_details = await do_request("https://api.igdb.com/v4/involved_companies", f"fields *; where id = {ic};", return_only_first=True)
            company_name = await do_request("https://api.igdb.com/v4/companies", f"fields name; where id = {involved_company_details.get('company')};", return_only_first=True)
            if involved_company_details.get("developer"):
                if company_name.get("name"):
                    developer_names.append(company_name.get("name"))
            elif involved_company_details.get("publisher"):
                if company_name.get("name"):
                    publisher_names.append(company_name.get("name"))
        if developer_names:
            developers = ", ".join(developer_names)
            publishers = ", ".join(publisher_names)

    genres = "N/A"
    if full_game_entry.get("genres"):
        genre_names = []
        for genre in full_game_entry.get("genres"):
            g = await do_request("https://api.igdb.com/v4/genres", f"fields *; where id = {genre};", return_only_first=True)
            if g.get("name"):
                genre_names.append(g.get("name"))
        if genre_names:
            genres = ", ".join(genre_names)
    
    rel_year = None
    if full_game_entry.get("first_release_date"):
        rel_year = datetime.fromtimestamp(full_game_entry.get("first_release_date")).year

    return {
        "developer": developers,
        "publisher": publishers,
        "series": series,
        "genres": genres,
        "release_year": rel_year,
        "name": full_game_entry.get("name")
    }


@router.get("/igdb/getgame_v2")
async def igdb_getgame_v2(id: int):
    query = f"""
    fields
        name,
        first_release_date,
        collections.name,
        genres.name,
        involved_companies.developer,
        involved_companies.publisher,
        involved_companies.company.name;
    where id = {id};
    """

    game = await do_request(
        "https://api.igdb.com/v4/games",
        query,
        return_only_first=True
    )

    # series
    series = "N/A"
    if game.get("collections"):
        series = ", ".join(c["name"] for c in game["collections"] if c.get("name"))

    # genres
    genres = "N/A"
    if game.get("genres"):
        genres = ", ".join(g["name"] for g in game["genres"] if g.get("name"))

    # developers / publishers
    developers = []
    publishers = []

    for ic in game.get("involved_companies", []):
        company = ic.get("company", {})
        name = company.get("name")

        if not name:
            continue

        if ic.get("developer"):
            developers.append(name)

        if ic.get("publisher"):
            publishers.append(name)

    developers = ", ".join(developers) if developers else "N/A"
    publishers = ", ".join(publishers) if publishers else "N/A"

    rel_year = None
    if game.get("first_release_date"):
        rel_year = datetime.fromtimestamp(
            game["first_release_date"]
        ).year

    return {
        "developer": developers,
        "publisher": publishers,
        "series": series,
        "genres": genres,
        "release_year": rel_year,
        "name": game.get("name")
    }
