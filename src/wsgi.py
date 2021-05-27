import os
with open('testing.txt', 'w') as file:
    file.write('working')

    path = os.path.dirname(os.path.realpath(__file__))
    file.write(f'{path}\n')


from rover_polar import create_app

app = create_app()

if __name__ == '__main__':
    app.run(debug=True)
