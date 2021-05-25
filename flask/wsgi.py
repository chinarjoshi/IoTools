from datetime import datetime
from flask import abort, Flask, jsonify, render_template, request

import csv


def user_validation(key_in):
    if key_in == KEY:
        return True
    return False


KEY = "6,wD-Ak]^wzWe@G"  # expected key for verification

app = Flask(__name__)


@app.route("/")
def home():
    return render_template("home.html")


@app.route("/data", methods=["GET"])
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
    with open("static/database/data.csv") as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=",")
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
    data = {
        "timestamps": times,
        "probeTemp": probe_temps,
        "bmeTemp": bme_temps,
        "bmeHumid": bme_humidity,
        "bmeAlt": bme_altitudes,
        "bmeAir": bme_airpress,
    }
    data_json = jsonify({"data": data})
    return data_json


# Update the database
@app.route("/update", methods=["POST"])
def update():
    r = request.form
    # Validate the user
    if r.get("key"):
        if not user_validation(r.get("key")):
            print("Couldn't validate user")
            abort(403)
    if r.get("probeTemp"):
        probeTemp = r.get("probeTemp")
    else:
        abort(404)
    if r.get("bmeTemp"):
        bmeTemp = r.get("bmeTemp")
    else:
        abort(404)
    if r.get("bmeHumid"):
        bmeHumid = r.get("bmeHumid")
    else:
        abort(404)
    if r.get("bmeAlt"):
        bmeAlt = r.get("bmeAlt")
    else:
        abort(404)
    if r.get("bmeAir"):
        bmeAir = r.get("bmeAir")
    else:
        abort(404)
    timestamp = datetime.now().strftime(
        "%d/%m/%Y %H:%M:%S"
    )  # get the current time of the data being pushed

    with open("static/database/data.csv", "a") as csv_file:
        csv_file.write(
            "{}, {}, {}, {}, {}, {}\n".format(
                timestamp, probeTemp, bmeTemp, bmeHumid, bmeAlt, bmeAir
            )
        )

    return "Successfully updated the data.", 201


# Reset database
@app.route("/reset", methods=["POST"])
def reset():
    r = request.form
    # Validate the user
    if r.get("key"):
        if not user_validation(r.get("key")):
            abort(403)
    # Truncate the current csv file
    with open("static/database/data.csv", "w") as csv_file:
        csv_file.truncate()
        csv_file.write(
            "Timestamp,Probe Temperature [C],BME Temperature [C],BME Humidity [%],BME Altitude [m],BME Air Pressure [hPA]\n"
        )
    return "Successfully reset the data.", 201


if __name__ == "__main__":
    app.run(debug=True)
