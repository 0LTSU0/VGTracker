from fastapi import FastAPI, Depends, HTTPException, Body, FastAPI, Request
from fastapi.responses import HTMLResponse
from fastapi.templating import Jinja2Templates
from fastapi.staticfiles import StaticFiles
from sqlalchemy.orm import Session

from .database import Base, engine, get_db
from .models import User, Entry, Platform
from .auth import hash_password, verify_password, create_token, get_current_user

app = FastAPI()
templates = Jinja2Templates(directory="templates")

app.mount("/covers", StaticFiles(directory="covers"), name="covers")

Base.metadata.create_all(bind=engine)


# MARK: API endpoints

@app.post("/register")
def register(
    email: str = Body(),
    username: str = Body(),
    password: str = Body(),
    db: Session = Depends(get_db)
):
    existing = db.query(User).filter(User.email == email or User.username == username).first()

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


@app.post("/login")
def login(
    email: str = Body(),
    username: str = Body(),
    password: str = Body(),
    db: Session = Depends(get_db)
):
    user = db.query(User).filter(User.email == email or User.username == username).first()

    if not user:
        raise HTTPException(401, "Invalid credentials")

    if not verify_password(password, user.password_hash):
        raise HTTPException(401, "Invalid credentials")

    token = create_token(user.id)
    return {"access_token": token}


@app.post("/entries")
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


@app.get("/entries")
def list_entries(
    user = Depends(get_current_user),
    db: Session = Depends(get_db)
):
    entries = db.query(Entry).filter(
        Entry.user_id == user.id
    ).all()

    return [x.__dict__ for x in entries]


@app.post("/platforms")
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


@app.get("/platforms")
def list_entries(
    user = Depends(get_current_user),
    db: Session = Depends(get_db)
):
    plats = db.query(Platform).filter(
        Platform.user_id == user.id
    ).all()
    return [x.__dict__ for x in plats]


# MARK: HTML endpoints
@app.get("/login", response_class=HTMLResponse)
def login_page(request: Request):
    return templates.TemplateResponse("login.html", {"request": request})


@app.get("/register", response_class=HTMLResponse)
def register_page(request: Request):
    return templates.TemplateResponse("register.html", {"request": request})


@app.get("/dashboard", response_class=HTMLResponse)
def dashboard_page(request: Request):
    return templates.TemplateResponse("dashboard.html", {"request": request})

