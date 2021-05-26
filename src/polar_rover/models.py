from datetime import datetime
from itsdangerous import TimedJSONWebSignatureSerializer as Serializer
from flask import current_app
from polar_rover import db, login_manager
from flask_login import UserMixin


@login_manager.user_loader
def load_user(user_id):
    return Session.query.get(int(user_id))


class Session(db.Model, UserMixin):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Colum(db.String(20), unique=True, nullable=False)
    password = db.Column(db.String(60), nullable=False)
    data = db.relationship('Data', lazy=True)


class Data(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    # TODO put csv fields as colmnn names.
