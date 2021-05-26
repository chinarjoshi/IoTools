from flask import Flask


app = Flask(__name__)
app.config['SECRET_KEY'] = '27bee976091724461b9bb1215ab2a696'

from polar_rover import routes
