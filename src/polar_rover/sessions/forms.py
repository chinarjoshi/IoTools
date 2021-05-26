from flask_wtf import FlaskForm
from wtforms import BooleanField, PasswordField, StringField, SubmitField
from wtforms.validators import DataRequired, Length


class LoginForm(FlaskForm):
    name = StringField('Session ID',
                           validators=[DataRequired(), Length(min=5, max=50)])
    password = PasswordField('Session Password',
                           validators=[DataRequired(), Length(min=3, max=30)])
    confirm_password = PasswordField('Confirm Password',
                                     validators=[DataRequired(), Length(min=3, max=30)])
    remember = BooleanField('Remember Me')
    submit = SubmitField('Login to a Session')


class RegistrationForm(FlaskForm):
    name = StringField('Session ID',
                           validators=[DataRequired(), Length(min=5, max=50)])
    password = PasswordField('Session Password',
                           validators=[DataRequired(), Length(min=3, max=30)])
    confirm_password = PasswordField('Confirm Password',
                                     validators=[DataRequired(), Length(min=3, max=30)])
    submit = SubmitField('Register a Session')

    def validate_name(self, name):
        session = Session.query.filter_by(name=name.data).first()
        if session:
            raise ValidationError('Session ID is taken.')
