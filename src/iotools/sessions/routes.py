from flask import (Blueprint, Response, flash, redirect, render_template,
                   url_for)
from flask_login import current_user, login_user, logout_user
from iotools import bcrypt, db
from iotools.models import Session
from iotools.sessions.forms import LoginForm, RegistrationForm

sessions = Blueprint('sessions', __name__)


@sessions.route('/enter', methods=['GET', 'POST'])
def enter() -> Response:
    if current_user.is_authenticated:
        return redirect(url_for('main.home'))
    form = LoginForm()
    if form.validate_on_submit():
        session = Session.query.first()
        if session and bcrypt.check_password_hash(session.password, form.password.data):
            login_user(session, remember=form.remember.data)
            return redirect(url_for('main.home'))
        else:
            flash('Login Unsuccessful', 'danger')
    return render_template('login.html', title='Login', form=form)


@sessions.route('/exit')
def exit() -> Response:
    logout_user()
    return redirect(url_for('main.home'))


@sessions.route('/create', methods=['GET', 'POST'])
def create() -> Response:
    if current_user.is_authenticated:
        return redirect(url_for('main.home'))
    form = RegistrationForm()
    if form.validate_on_submit():
        hashed_password = bcrypt.generate_password_hash(form.password.data).decode('utf-8')
        session = Session(name=form.username.data, password=hashed_password)
        db.session.add(session)
        db.session.commit()
        flask('Account successfully created', 'success')
    return render_template('login.html', title='Register', form=form)
