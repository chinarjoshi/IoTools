from flask_login import UserMixin

from rover_polar import db, login_manager


@login_manager.user_loader
def load_user(user_id):
    return Session.query.get(int(user_id))


class Session(db.Model, UserMixin):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(20), unique=True, nullable=False)
    password = db.Column(db.String(60), nullable=False)
    # One (session) to many (datapoints) relationship
    data = db.relationship('Data', lazy=True)


class Data(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    timestamp = db.Column(db.String(20))
    probe_temp = db.Column(db.Integer)
    bme_temp = db.Column(db.Integer)
    bme_humidity = db.Column(db.Integer)
    bme_alt = db.Column(db.Integer)
    bme_air_pressure = db.Column(db.Integer)
