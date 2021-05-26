from flask import render_template, redirect, request, url_for, Blueprint, flash
from flask_login import login_user, current_user, logout_user, login_required
from polar_rover import db, bcrypt
from polar_rover.models import Session, Data
from polar_rover.sessions.forms import LoginForm, RegistrationForm

users = Blueprint('users', __name__)

@users.route('/login', methods=['GET', 'POST'])
def login():
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


@users.route('/register', methods=['GET', 'POST'])
def register():
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
