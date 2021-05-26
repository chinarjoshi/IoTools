from dataclasses import dataclass

@dataclass(frozen=True)
class Output():
    """Inmutable class for containing return message and code."""
    message: str
    return_code: int

CSV_ROUTE = '../static/data/data.csv'
KEY = '6,wD-Ak]^wzWe@G'  # expected key for verification
