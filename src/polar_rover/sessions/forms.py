from flask_wtf import FlaskForm
from wtforms import StringField, PasswordField, SubmitField, BooleanField
from wtforms.validators import DataRequired, Length


class LoginForm(FlaskForm):
    id = StringField('Session ID',
                           validators=[DataRequired(), Length(min=5, max=50)])
    password = PasswordField('Session Password',
                           validators=[DataRequired(), Length(min=3, max=30)])
    confirm_password = PasswordField('Confirm Password',
                                     validators=[DataRequired(), Length(min=3, max=30)])
    remember = BooleanField('Remember Me')
    submit = SubmitField('Login to a Session')

class RegistrationForm(FlaskForm):
    id = StringField('Session ID',
                           validators=[DataRequired(), Length(min=5, max=50)])
    password = PasswordField('Session Password',
                           validators=[DataRequired(), Length(min=3, max=30)])
    confirm_password = PasswordField('Confirm Password',
                                     validators=[DataRequired(), Length(min=3, max=30)])
    submit = SubmitField('Register a Session')
