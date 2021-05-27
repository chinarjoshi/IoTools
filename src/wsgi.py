
from testing.test import test

test()

from rover_polar import create_app

app = create_app()

if __name__ == '__main__':
    app.run(debug=True)
