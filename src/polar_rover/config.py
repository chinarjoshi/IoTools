import os


class Config:
    SECRET_KEY = 'c044a8a1683be4c503f66f8d716429e4'
    SQLALCHEMY_DATABASE_URI = os.environ.get('SQLALCHEMY_DATABASE_URI')
