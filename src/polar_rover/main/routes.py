import importlib.resources as pkg_resources
from dataclasses import dataclass
from flask import render_template, request, Blueprint

main = Blueprint('main', __name__)


@dataclass(frozen=True)
class Output():
    """Inmutable class for containing return message and code."""
    message: str
    return_code: int

# CSV file is imported from package route as file stream
CSV_STREAM = pkg_resources.open_text(polar_rover, 'data.csv')
KEY = '6,wD-Ak]^wzWe@G'  # expected key for verification

@main.route('/')
@main.route('/home')
def home():
    """Home page."""
    return render_template('home.html')


@main.route('/about')
def about():
    """About page."""
    return render_template('about.html', title='About')


@main.route('/instructions')
def instructions():
    """Instructions page."""
    return render_template('instructions.html', title='Instructions')

@main.route('/data')
def data() -> Response:
    """Read the data from the cvs file.

    Returns
        data_json (Response): json containing the information of the file
    """
    return jsonify({'data': pd.read_csv(CSV_STREAM).to_dict()})


@main.route('/update', methods=['POST'])
def update() -> Output:
    """Update csv values from request."""
    values = {}
    for field in 'key', 'probeTemp', 'bmeTemp', 'bmeHumid', 'bmeAlt', 'bmeAir':
        try:
            values[field] = request.form[field]
        except:
            abort(404)
    with open(CSV_STREAM, 'a') as csv_file:
        csv_file.write(', '.join(values.items()))
    return Output('Successfully reset the data.', 201)


@main.route('/reset', methods=['POST'])
def reset() -> Output:
    """Reset database."""
    if request.form.get('key') != KEY:
        abort(403)
    with open(CSV_STREAM, 'w') as csv_file:
        csv_file.write('Timestamp,Probe Temperature [C],BME Temperature [C],BME Humidity [%],BME Altitude [m],BME Air Pressure [hPA]\n')
    return Output('Successfully reset the data.', 201)
