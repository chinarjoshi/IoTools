import os
import pandas as pd
from dataclasses import dataclass


dir, _ = os.path.split(__file__)

CSV_STREAM = os.path.join(dir, '..', 'data.csv')
KEY = '6,wD-Ak]^wzWe@G'  # expected key for verification

@dataclass(frozen=True)
class Output():
    """Inmutable class for containing return message and code."""
    message: str
    return_code: int


def temperature_increasing(stream: str) -> str:
    """"Check average of data row and compare it to first datapoint."""
    df = pd.read_csv(stream)
    average = df['Probe Temperature [C]'].mean()
    initial = df['Probe Temperature [C]'].iloc[0]

    return 'Increasing' if average > initial else 'Decreasing'
