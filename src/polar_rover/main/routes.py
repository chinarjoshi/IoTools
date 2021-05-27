import csv

from flask import Blueprint, Response, abort, jsonify, render_template, request
from polar_rover.main.utils import CSV_STREAM, KEY, Output

main = Blueprint('main', __name__)


@main.route('/')
@main.route('/home')
def home() -> Response:
    """Home page."""
    return render_template('home.html')


@main.route('/about')
def about() -> Response:
    """About page."""
    return render_template('about.html', title='About')


@main.route('/instructions')
def instructions() -> Response:
    """Instructions page."""
    return render_template('instructions.html', title='Instructions')


@main.route('/data')
def get_data():
    """
    Read the data from the cvs file.
    
    Returns 
        data_json (json) : a json containing the information of the file
    """
    times = []
    probe_temps = []
    bme_temps = []
    bme_humidity = []
    bme_altitudes = []
    bme_airpress = []
    with open(CSV_STREAM) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        line_count = 0
        for row in csv_reader:
            if line_count != 0:
                times.append(row[0])
                probe_temps.append(float(row[1]))
                bme_temps.append(float(row[2]))
                bme_humidity.append(float(row[3]))
                bme_altitudes.append(float(row[4]))
                bme_airpress.append(float(row[5]))
            line_count += 1
    data = {'timestamps' : times, 'probeTemp' : probe_temps, 'bmeTemp' : bme_temps,
            'bmeHumid' : bme_humidity, 'bmeAlt': bme_altitudes, 'bmeAir': bme_airpress}
    data_json = jsonify({'data' : data})
    return data_json

@main.route('/update', methods=['POST'])
def update():
    r = request.form
    # Validate the user
    if r.get('key'):
        if not user_validation(r.get('key')):
            print("Couldn't validate user")
            abort(403)
    if r.get('probeTemp'):
        probeTemp = r.get('probeTemp')
    else:
        abort(404)
    if r.get('bmeTemp'):
        bmeTemp = r.get('bmeTemp')
    else:
        abort(404)
    if r.get('bmeHumid'):
        bmeHumid = r.get('bmeHumid')
    else:
        abort(404)
    if r.get('bmeAlt'):
        bmeAlt = r.get('bmeAlt')
    else:
        abort(404)
    if r.get('bmeAir'):
        bmeAir = r.get('bmeAir')
    else:
        abort(404)
    timestamp = datetime.now().strftime("%d/%m/%Y %H:%M:%S") # get the current time of the data being pushed

    with open(CSV_STREAM, 'a') as csv_file:
        csv_file.write("{}, {}, {}, {}, {}, {}\n".format(timestamp, probeTemp, bmeTemp, bmeHumid, bmeAlt, bmeAir))
    
    return Output("Successfully updated the data.", 201)


@main.route('/reset', methods=['POST'])
def reset() -> Output:
    """Reset database."""
    if request.form.get('key') != KEY:
        abort(403)
    with open(CSV_STREAM, 'w') as csv_file:
        csv_file.write('Timestamp,Probe Temperature [C],BME Temperature [C],BME Humidity [%],BME Altitude [m],BME Air Pressure [hPA]\n')
    return Output('Successfully reset the data.', 201)
