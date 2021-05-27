import os
import importlib

print('\n\n\nWORKING\n\n\n')
print(os.path.dirname(os.path.realpath(__file__)))


app = importlib.import_module('rover_polar')
if __name__ == '__main__':
    getattr(app, 'create_app')()
