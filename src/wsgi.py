import os

print('\n\n\nWORKING\n\n\n')
print(os.path.dirname(os.path.realpath(__file__)))


from rover_polar import create_app

app = create_app()

if __name__ == '__main__':
    app.run(debug=True)
