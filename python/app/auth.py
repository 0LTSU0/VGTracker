from jose import jwt
from passlib.context import CryptContext
from fastapi import Header, HTTPException, Cookie, Depends
from sqlalchemy.orm import Session
from .models import User
from .database import SessionLocal, get_db

SECRET = "supersecuresecret"
ALGORITHM = "HS256"

pwd_context = CryptContext(schemes=["bcrypt"])


def hash_password(password):
    return pwd_context.hash(password)


def verify_password(password, hashed):
    return pwd_context.verify(password, hashed)


def create_token(user_id):
    data = {"user_id": user_id}
    token = jwt.encode(data, SECRET, algorithm=ALGORITHM)
    return token


def check_token(token):
    try:
        payload = jwt.decode(token, SECRET, algorithms=[ALGORITHM])
        user_id = payload.get("user_id")
        db = SessionLocal()
        user = db.query(User).filter(User.id == user_id).first()
        if not user:
            return False
    except:
        return False
    return True


def get_current_user(token: str | None = Cookie(default=None), db: Session = Depends(get_db)):
    try:
        payload = jwt.decode(token, SECRET, algorithms=[ALGORITHM])
    except:
        raise HTTPException(status_code=401)

    user_id = payload.get("user_id")

    user = db.query(User).filter(User.id == user_id).first()

    if not user:
        raise HTTPException(status_code=401)

    return user
