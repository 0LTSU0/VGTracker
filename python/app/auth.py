from jose import jwt
from passlib.context import CryptContext
from fastapi import Header, HTTPException, Cookie, Depends
from fastapi.security import HTTPAuthorizationCredentials, HTTPBearer
from sqlalchemy.orm import Session
from .models import User
from .database import SessionLocal, get_db

SECRET = "supersecuresecret"
ALGORITHM = "HS256"

pwd_context = CryptContext(schemes=["bcrypt"])
security = HTTPBearer(auto_error=False)

def hash_password(password):
    return pwd_context.hash(password)


def verify_password(password, hashed):
    return pwd_context.verify(password, hashed)


def create_token(user_id):
    data = {"user_id": user_id}
    token = jwt.encode(data, SECRET, algorithm=ALGORITHM)
    return token


def check_token(token):
    db = SessionLocal()
    try:
        payload = jwt.decode(token, SECRET, algorithms=[ALGORITHM])
        user_id = payload.get("user_id")
        user = db.query(User).filter(User.id == user_id).first()
        if not user:
            return False
    except:
        return False
    finally:
        db.close()
    return True


def get_current_user(
        creds: HTTPAuthorizationCredentials | None = Depends(security),
        token: str | None = Cookie(default=None), 
        db: Session = Depends(get_db)
    ):
    
    if creds is not None: # check for bearer token
        token = creds.credentials

    if not token:
        raise HTTPException(status_code=401, detail="Not authenticated")
    
    try:
        payload = jwt.decode(token, SECRET, algorithms=[ALGORITHM])
    except:
        raise HTTPException(status_code=401)

    user_id = payload.get("user_id")

    user = db.query(User).filter(User.id == user_id).first()

    if not user:
        raise HTTPException(status_code=401)

    return user
