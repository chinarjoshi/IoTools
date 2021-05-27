import os
dir, _ = os.path.split(__file__)

file_path = os.path.join(dir, '..', 'Procfile')
with open(file_path) as file:
    file.read()




from rover_polar import create_app

app = create_app()

if __name__ == '__main__':
    app.run(debug=True)
