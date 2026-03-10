from sqlalchemy import Column, Integer, String, ForeignKey, Text, Boolean, UniqueConstraint, Float
from sqlalchemy.orm import relationship
from .database import Base


class User(Base):
    __tablename__ = "users"

    id = Column(Integer, primary_key=True)
    username = Column(String, unique=True)
    email = Column(String, unique=True)
    password_hash = Column(String)

    platforms = relationship("Platform", back_populates="user", cascade="all, delete")
    entries = relationship("Entry", back_populates="user", cascade="all, delete")


class Platform(Base):
    __tablename__ = "platforms"
    __table_args__ = (
        UniqueConstraint("user_id", "name"), # to make sure user doesn't have multiple PS5s
    )

    id = Column(Integer, primary_key=True)
    name = Column(String, nullable=False) # Platform name e.g. PS5
    supports_platinum = Column(Boolean) # If this platform has platinum trophies

    user_id = Column(Integer, ForeignKey("users.id"), nullable=False) # who this platform belongs to

    user = relationship("User", back_populates="platforms")
    entries = relationship("Entry", back_populates="platform", cascade="all, delete")


class Entry(Base):
    __tablename__ = "entries"

    id = Column(Integer, primary_key=True)

    title = Column(String, nullable=False)
    notes = Column(Text)

    #misc metadata fields
    status = Column(String) # In progress, Completed, Not started
    release_year = Column(Integer)
    rating = Column(Integer)
    developer = Column(String)
    publisher = Column(String)
    genres = Column(String)
    series = Column(String)
    cover_path = Column(String)
    platinumed = Column(Boolean)
    price = Column(Float)

    user_id = Column(Integer, ForeignKey("users.id"), nullable=False) # whose entry this is
    platform_id = Column(Integer, ForeignKey("platforms.id"), nullable=False) # what platform this game is for

    user = relationship("User", back_populates="entries")
    platform = relationship("Platform", back_populates="entries")