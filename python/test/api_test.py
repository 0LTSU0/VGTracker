import random

import pytest
from fastapi.testclient import TestClient
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from urllib.parse import quote

from app.main import app
from app.database import Base, get_db

from .test_helpers import *

TEST_DATABASE_URL = "sqlite:///./test.db"

engine = create_engine(TEST_DATABASE_URL, connect_args={"check_same_thread": False})
TestingSessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)



def override_get_db():
    db = TestingSessionLocal()
    try:
        yield db
    finally:
        db.close()


@pytest.fixture(scope="module", autouse=True)
def setup_db():
    Base.metadata.create_all(bind=engine)
    yield
    Base.metadata.drop_all(bind=engine)


@pytest.fixture(scope="module")
def client():
    app.dependency_overrides[get_db] = override_get_db
    with TestClient(app) as c:
        yield c
    app.dependency_overrides.clear()


@pytest.fixture(scope="module")
def logged_in_client(client):
    username = random_string(10)
    print("logged_in_client creating user", username)
    res = client.post("/api/register", json={
        "username": username,
        "email": username + "@example.com",
        "password": "securepassword123",
    })
    assert res.status_code == 200
    res = client.post("/api/login", json={
        "email": username,
        "password": "securepassword123",
    })
    assert res.status_code == 200
    # TestClient keeps the token cookie automatically after login
    return client

@pytest.fixture(scope="module")
def logged_in_client_2():
    with TestClient(app) as c:
        username = random_string(10)
        c.post("/api/register", json={
            "username": username,
            "email": username + "@example.com",
            "password": "securepassword123",
        })
        c.post("/api/login", json={
            "email": username,
            "password": "securepassword123",
        })
        yield c

# MARK: AUTH
def test_register_create_user(client):
    response = client.post("/api/register", json={
        "username": "testuser",
        "email": "test@example.com",
        "password": "securepassword123",
    })
    assert response.status_code == 200


def test_register_existing_user(client):
    response = client.post("/api/register", json={
        "username": "testuser123",
        "email": "test123@example.com",
        "password": "securepassword123",
    })
    assert response.status_code == 200 # first time should be ok
    response = client.post("/api/register", json={
        "username": "testuser123",
        "email": "notthesame@example.com",
        "password": "securepassword123",
    })
    assert response.status_code == 400
    assert "User/Email already registered" in str(response.json())
    response = client.post("/api/register", json={
        "username": "notthesame",
        "email": "test123@example.com",
        "password": "securepassword123",
    })
    assert response.status_code == 400
    assert "User/Email already registered" in str(response.json())


# MARK: PLATFORMS
def test_get_platforms_empty(logged_in_client):
    """Verify there are no platforms for new user"""
    response = logged_in_client.get("/api/platforms")
    assert response.status_code == 200
    assert response.json() == []

def test_create_and_get_platforms(logged_in_client):
    """Create platforms and verify they are returned upon doing GET"""
    new_platform = create_random_platform()
    res = logged_in_client.post("/api/platforms", json=new_platform)
    assert res.status_code == 200
    assert new_platform.get("name") == res.json().get("name")
    assert new_platform.get("supports_platinum") == res.json().get("supports_platinum")
    new_platform2 = create_random_platform()
    res = logged_in_client.post("/api/platforms", json=new_platform2)
    assert res.status_code == 200
    assert new_platform2.get("name") == res.json().get("name")
    assert new_platform2.get("supports_platinum") == res.json().get("supports_platinum")
    
    res = logged_in_client.get("/api/platforms")
    assert res.status_code == 200
    assert len(res.json()) == 2
    for plat in res.json():
        assert plat.get("name") == new_platform.get("name") or plat.get("name") == new_platform2.get("name")
        assert plat.get("supports_platinum") == new_platform.get("supports_platinum") or plat.get("supports_platinum") == new_platform2.get("supports_platinum")

def test_create_existing_platform(logged_in_client):
    """Verify one user cannot have multiple platforms with same name"""
    new_platform = create_random_platform()
    res = logged_in_client.post("/api/platforms", json=new_platform)
    assert res.status_code == 200
    assert new_platform.get("name") == res.json().get("name")
    assert new_platform.get("supports_platinum") == res.json().get("supports_platinum")
    res = logged_in_client.post("/api/platforms", json=new_platform)
    assert res.status_code == 406

def test_same_platform_name_for_2_users(logged_in_client, logged_in_client_2):
    """Make sure two users can have a platform with the same name"""
    new_platform = create_random_platform()
    res = logged_in_client.post("/api/platforms", json=new_platform)
    assert res.status_code == 200
    assert new_platform.get("name") == res.json().get("name")
    assert new_platform.get("supports_platinum") == res.json().get("supports_platinum")
    res = logged_in_client_2.post("/api/platforms", json=new_platform)
    assert res.status_code == 200
    assert new_platform.get("name") == res.json().get("name")
    assert new_platform.get("supports_platinum") == res.json().get("supports_platinum")
    res = logged_in_client_2.post("/api/platforms", json=new_platform)
    assert res.status_code == 406

def test_platform_without_name(logged_in_client):
    """Make sure platform needs to have a name to be created"""
    new_platform = create_random_platform()
    new_platform["name"] = None
    res = logged_in_client.post("/api/platforms", json=new_platform)
    assert res.status_code == 406

def test_platform_without_supports_platinum(logged_in_client):
    """Make sure platform can be created without supports_platinum flag"""
    new_platform = create_random_platform()
    new_platform["supports_platinum"] = None
    res = logged_in_client.post("/api/platforms", json=new_platform)
    assert res.status_code == 200
    assert res.json().get("name") == new_platform.get("name")
    assert res.json().get("supports_platinum") == new_platform.get("supports_platinum")

# MARK: ENTRIES
def test_create_entry(logged_in_client):
    """Test valid and fully filled entry for valid platform"""
    new_platform = create_random_platform()
    platform_res = logged_in_client.post("/api/platforms", json=new_platform)
    assert platform_res.status_code == 200
    new_entry = create_random_game_entry(platform_res.json().get("id"))
    res = logged_in_client.post("/api/entries", json=new_entry)
    created_entry = res.json()
    assert res.status_code == 200
    assert compare_dict_common_key_values(new_entry, created_entry)


def test_create_entry_only_required_fields(logged_in_client):
    """Entry with only title and platform_id should be accepted — all other fields are optional"""
    platform_res = logged_in_client.post("/api/platforms", json=create_random_platform())
    assert platform_res.status_code == 200
    res = logged_in_client.post("/api/entries", json={
        "title": random_string(10),
        "platform_id": platform_res.json().get("id"),
    })
    assert res.status_code == 200


def test_create_entry_missing_title(logged_in_client):
    """Entry without title must be rejected"""
    platform_res = logged_in_client.post("/api/platforms", json=create_random_platform())
    assert platform_res.status_code == 200
    entry = create_random_game_entry(platform_res.json().get("id"))
    del entry["title"]
    res = logged_in_client.post("/api/entries", json=entry)
    assert res.status_code == 406


def test_create_entry_missing_platform(logged_in_client):
    """Entry without platform_id must be rejected"""
    entry = create_random_game_entry(99999)
    del entry["platform_id"]
    res = logged_in_client.post("/api/entries", json=entry)
    assert res.status_code == 406


def test_create_entry_nonexistent_platform(logged_in_client):
    """Entry referencing a platform that does not exist must be rejected"""
    entry = create_random_game_entry(99999)
    res = logged_in_client.post("/api/entries", json=entry)
    assert res.status_code == 406


def test_create_entry_other_users_platform(logged_in_client, logged_in_client_2):
    """User must not be able to create an entry on another user's platform"""
    platform_res = logged_in_client.post("/api/platforms", json=create_random_platform())
    assert platform_res.status_code == 200
    other_platform_id = platform_res.json().get("id")
    entry = create_random_game_entry(other_platform_id)
    res = logged_in_client_2.post("/api/entries", json=entry)
    assert res.status_code == 406


def test_two_users_same_entry(logged_in_client, logged_in_client_2):
    """Two different users can have identical-looking entries"""
    platform_1 = logged_in_client.post("/api/platforms", json=create_random_platform()).json()
    platform_2 = logged_in_client_2.post("/api/platforms", json=create_random_platform()).json()
    title = random_string(20)
    entry_1 = create_random_game_entry(platform_1.get("id"))
    entry_1["title"] = title
    entry_2 = create_random_game_entry(platform_2.get("id"))
    entry_2["title"] = title
    assert logged_in_client.post("/api/entries", json=entry_1).status_code == 200
    assert logged_in_client_2.post("/api/entries", json=entry_2).status_code == 200


def test_get_entries_returns_only_own(logged_in_client, logged_in_client_2):
    """GET /api/entries must only return the authenticated user's own entries"""
    platform_1 = logged_in_client.post("/api/platforms", json=create_random_platform()).json()
    platform_2 = logged_in_client_2.post("/api/platforms", json=create_random_platform()).json()
    logged_in_client.post("/api/entries", json=create_random_game_entry(platform_1.get("id")))
    logged_in_client_2.post("/api/entries", json=create_random_game_entry(platform_2.get("id")))

    res_1 = logged_in_client.get("/api/entries")
    res_2 = logged_in_client_2.get("/api/entries")
    assert res_1.status_code == 200
    assert res_2.status_code == 200
    ids_1 = {e["id"] for e in res_1.json()}
    ids_2 = {e["id"] for e in res_2.json()}
    assert ids_1.isdisjoint(ids_2), "Users must not see each other's entries"


def test_get_entries_filter_by_platform(logged_in_client):
    """GET /api/entries?platform_id=X must only return entries for that platform"""
    p1 = logged_in_client.post("/api/platforms", json=create_random_platform()).json()
    p2 = logged_in_client.post("/api/platforms", json=create_random_platform()).json()
    logged_in_client.post("/api/entries", json=create_random_game_entry(p1.get("id")))
    logged_in_client.post("/api/entries", json=create_random_game_entry(p1.get("id")))
    logged_in_client.post("/api/entries", json=create_random_game_entry(p2.get("id")))

    res = logged_in_client.get(f"/api/entries?platform_id={p1.get('id')}")
    assert res.status_code == 200
    assert all(e["platform_id"] == p1.get("id") for e in res.json())


def test_update_entry(logged_in_client):
    """PUT must update provided fields and leave others unchanged"""
    platform = logged_in_client.post("/api/platforms", json=create_random_platform()).json()
    entry = logged_in_client.post("/api/entries", json=create_random_game_entry(platform.get("id"))).json()
    new_title = random_string(20)
    res = logged_in_client.put("/api/entries", json={"id": entry["id"], "title": new_title})
    assert res.status_code == 200
    assert res.json().get("title") == new_title
    assert res.json().get("platform_id") == entry["platform_id"]


def test_update_entry_missing_id(logged_in_client):
    """PUT without entry id must be rejected"""
    res = logged_in_client.put("/api/entries", json={"title": random_string(10)})
    assert res.status_code == 406


def test_update_entry_not_found(logged_in_client):
    """PUT with a nonexistent entry id must return 404"""
    res = logged_in_client.put("/api/entries", json={"id": 99999, "title": random_string(10)})
    assert res.status_code == 404


def test_update_entry_other_users_entry(logged_in_client, logged_in_client_2):
    """User must not be able to update another user's entry"""
    platform = logged_in_client.post("/api/platforms", json=create_random_platform()).json()
    entry = logged_in_client.post("/api/entries", json=create_random_game_entry(platform.get("id"))).json()
    res = logged_in_client_2.put("/api/entries", json={"id": entry["id"], "title": random_string(10)})
    assert res.status_code == 404


def test_update_entry_to_other_users_platform(logged_in_client, logged_in_client_2):
    """User must not be able to move an entry onto another user's platform"""
    own_platform = logged_in_client.post("/api/platforms", json=create_random_platform()).json()
    other_platform = logged_in_client_2.post("/api/platforms", json=create_random_platform()).json()
    entry = logged_in_client.post("/api/entries", json=create_random_game_entry(own_platform.get("id"))).json()
    res = logged_in_client.put("/api/entries", json={"id": entry["id"], "platform_id": other_platform.get("id")})
    assert res.status_code == 406


def test_delete_entry(logged_in_client):
    """DELETE must remove the entry and it must no longer appear in GET"""
    platform = logged_in_client.post("/api/platforms", json=create_random_platform()).json()
    entry = logged_in_client.post("/api/entries", json=create_random_game_entry(platform.get("id"))).json()
    res = logged_in_client.delete(f"/api/entries/{entry['id']}")
    assert res.status_code == 200
    entries = logged_in_client.get("/api/entries").json()
    assert not any(e["id"] == entry["id"] for e in entries)


def test_delete_entry_not_found(logged_in_client):
    """DELETE of nonexistent entry must return 404"""
    res = logged_in_client.delete("/api/entries/99999")
    assert res.status_code == 404


def test_delete_entry_other_users_entry(logged_in_client, logged_in_client_2):
    """User must not be able to delete another user's entry"""
    platform = logged_in_client.post("/api/platforms", json=create_random_platform()).json()
    entry = logged_in_client.post("/api/entries", json=create_random_game_entry(platform.get("id"))).json()
    res = logged_in_client_2.delete(f"/api/entries/{entry['id']}")
    assert res.status_code == 404
    entries = logged_in_client.get("/api/entries").json()
    assert any(e["id"] == entry["id"] for e in entries), "Entry must still exist after failed delete"


#MARK: IGDB
def test_igdb_search(logged_in_client):
    """Verify igdb can be used to search games"""
    res = logged_in_client.get(f"/api/igdb/search?name={quote("Uncharted", safe="")}")
    resdata = res.json()
    assert res.status_code == 200
    assert len(resdata) > 1
    for entry in resdata:
        print("IGDB search returned", entry.get("name"))

def test_post_entry_based_on_igdb_result(logged_in_client):
    """Search for game and use the returned IGDB serch result to create entry"""
    platform = logged_in_client.post("/api/platforms", json=create_random_platform()).json()
    res = logged_in_client.get(f"/api/igdb/search?name={quote("my winter car", safe="")}")
    resdata = res.json()
    assert res.status_code == 200
    assert resdata
    gamedatares = logged_in_client.get(f"/api/igdb/getgame_v2?id={resdata[0]["id"]}")
    gamedata = gamedatares.json()
    assert gamedatares.status_code == 200
    assert gamedata
    gamedata["platform_id"] = platform.get("id")
    gamedata["title"] = gamedata.pop("name")
    temp_cover_id = gamedata["cover_temp_key"]
    del gamedata["cover_temp_key"]
    create_game_res = logged_in_client.post("/api/entries", json=gamedata)
    assert create_game_res.status_code == 200
    
    # at this point cover art should be availbale with temp name until we commit it with the newly created entry id
    temp_img_res = logged_in_client.get(f"/covers/temp_{int(temp_cover_id)}.png")
    assert temp_img_res.status_code == 200
    assert temp_img_res.content != logged_in_client.get(f"/covers/placeholder.png")

    # after we commmit we should get the same image with the entry id as before
    res = logged_in_client.post(f"/api/entries/{create_game_res.json().get("id")}/cover/from_temp?key={int(temp_cover_id)}")
    assert res.status_code == 200
    res_final_image = logged_in_client.get(f"/covers/{create_game_res.json().get("id")}.png")
    assert res_final_image.status_code == 200
    assert res_final_image.content == temp_img_res.content
