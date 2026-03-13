from fastapi import FastAPI, Depends, HTTPException, Body, FastAPI, Request, Cookie, Query
from fastapi.responses import HTMLResponse, JSONResponse, RedirectResponse
from fastapi.templating import Jinja2Templates
from fastapi.staticfiles import StaticFiles
from sqlalchemy.orm import Session
from sqlalchemy import or_, and_

from .database import Base, engine, get_db
from .models import User, Entry, Platform
from .auth import hash_password, verify_password, create_token, get_current_user, check_token

app = FastAPI()
templates = Jinja2Templates(directory="templates")

app.mount("/covers", StaticFiles(directory="covers"), name="covers")
app.mount("/static", StaticFiles(directory="static"), name="static")

Base.metadata.create_all(bind=engine)


# MARK: API endpoints

@app.post("/api/register")
def register(
    email: str = Body(),
    username: str = Body(),
    password: str = Body(),
    db: Session = Depends(get_db)
):
    existing = db.query(User).filter(or_(User.email == email, User.username == username)).first()

    if existing:
        raise HTTPException(400, "User/Email already registered")

    user = User(
        email = email,
        username = username,
        password_hash = hash_password(password)
    )

    db.add(user)
    db.commit()
    db.refresh(user)

    return {"message": "user created"}


@app.post("/api/login")
def login(
    email: str = Body(None),
    password: str = Body(),
    db: Session = Depends(get_db)
):
    user = db.query(User).filter(
        or_(User.email == email, User.username == email)
    ).first()

    if not user or not verify_password(password, user.password_hash):
        raise HTTPException(401, "Invalid credentials")

    token = create_token(user.id)

    response = JSONResponse({"status": "ok", "user_id": user.id})

    response.set_cookie(
        key="token",
        value=token
        #httponly=True,
        #samesite="lax"
    )

    return response


@app.get("/logout")
def logout():
    response = RedirectResponse("/login")
    response.delete_cookie("token")
    return response


@app.post("/api/entries")
def create_entry(
    data: dict = Body(),
    user = Depends(get_current_user),
    db: Session = Depends(get_db)
):
    entry = Entry(**data)
    entry.user_id = user.id

    existing_platforms = db.query(Platform).filter(
        Platform.user_id == user.id
    ).all()
    if not any([x.id == entry.platform_id for x in existing_platforms]):
        raise HTTPException(406, "Platform missing or does not exist")

    try:
        db.add(entry)
        db.commit()
        db.refresh(entry)
    except Exception as e:
        db.rollback()
        raise HTTPException(406, str(e))

    return entry.__dict__


@app.put("/api/entries")
def update_entry(
    data: dict = Body(),
    user = Depends(get_current_user),
    db: Session = Depends(get_db)
):
    entry_id = data.get("id")
    if not entry_id:
        raise HTTPException(406, "Missing entry id in put request")
    
    entry = db.query(Entry).filter(
        Entry.id == entry_id,
        Entry.user_id == user.id
    ).first()
    if not entry:
        raise HTTPException(404, "Corresponding entry not found in DB")
    
    if "platform_id" in data:
        platform = db.query(Platform).filter(
            Platform.id == data["platform_id"],
            Platform.user_id == user.id
        ).first()
        if not platform:
            raise HTTPException(406, "No such platform")
        
    for key, value in data.items():
        setattr(entry, key, value)
    try:
        db.commit()
        db.refresh(entry)
    except Exception as e:
        db.rollback()
        raise HTTPException(406, str(e))
        
    return entry.__dict__


@app.delete("/api/entries/{entry_id}")
def delete_entry(
    entry_id: int,
    user = Depends(get_current_user),
    db: Session = Depends(get_db)
):
    entry = db.query(Entry).filter(
        Entry.id == entry_id,
        Entry.user_id == user.id
    ).first()

    if not entry:
        raise HTTPException(404, "Entry not found")

    try:
        db.delete(entry)
        db.commit()
    except Exception as e:
        db.rollback()
        raise HTTPException(406, str(e))

    return {"status": "deleted"}


@app.get("/api/entries")
def list_entries(
    platform_id: int | None = Query(default=None),
    user = Depends(get_current_user),
    db: Session = Depends(get_db)
):
    query = db.query(Entry).filter(
        Entry.user_id == user.id
    )

    if platform_id is not None:
        query = query.filter(Entry.platform_id == platform_id)

    entries = query.all()
    return [x.__dict__ for x in entries]


@app.post("/api/platforms")
def create_platform(
    data: dict = Body(),
    user = Depends(get_current_user),
    db: Session = Depends(get_db)
):
    platform = Platform(**data)
    platform.user_id = user.id
    try:
        db.add(platform)
        db.commit()
        db.refresh(platform)
    except Exception as e:
        db.rollback()
        raise HTTPException(406, str(e))
    return platform.__dict__


@app.get("/api/platforms")
def list_entries(
    user = Depends(get_current_user),
    db: Session = Depends(get_db)
):
    plats = db.query(Platform).filter(
        Platform.user_id == user.id
    ).all()
    return [x.__dict__ for x in plats]


# MARK: HTML endpoints
@app.get("/")
def root(token: str | None = Cookie(default=None)):
    if not token:
        return RedirectResponse("/login")
    try:
        success = check_token(token)
        if not success:
            return RedirectResponse("/login") 
    except:
        return RedirectResponse("/login")
    return RedirectResponse("/home")


@app.get("/login", response_class=HTMLResponse)
def login_page(request: Request):
    return templates.TemplateResponse("login.html", {"request": request})


@app.get("/register", response_class=HTMLResponse)
def register_page(request: Request):
    return templates.TemplateResponse("register.html", {"request": request})


@app.get("/home", response_class=HTMLResponse)
def home_page(request: Request, user = Depends(get_current_user)):
    return templates.TemplateResponse("home.html", {"request": request})

