print('\n\n\nWORKING 1\n\n\n')
from flask import Flask
print('\n\n\nWORKING 2\n\n\n')
from flask_sqlalchemy import SQLAlchemy
print('\n\n\nWORKING 2\n\n\n')
from flask_login import LoginManager
print('\n\n\nWORKING 3\n\n\n')
from flask_bcrypt import Bcrypt
print('\n\n\nWORKING 4\n\n\n')
from rover_polar.config import Config
print('\n\n\nWORKING 5\n\n\n')


db = SQLAlchemy()
bcrypt = Bcrypt()
login_manager = LoginManager()
login_manager.login_view = 'sessions.login'
login_manager.login_message_category = 'info'


def create_app(config_class=Config):
    app = Flask(__name__)
    app.config.from_object(Config)

    db.init_app(app)
    bcrypt.init_app(app)
    login_manager.init_app(app)

    from rover_polar.sessions.routes import sessions
    from rover_polar.main.routes import main
    app.register_blueprint(sessions)
    app.register_blueprint(main)

    return app
