import os
from dataclasses import dataclass


dir, _ = os.path.split(__file__)

CSV_STREAM = os.path.join(dir, '..', 'data.csv')
KEY = '6,wD-Ak]^wzWe@G'  # expected key for verification

@dataclass(frozen=True)
class Output():
    """Inmutable class for containing return message and code."""
    message: str
    return_code: int
