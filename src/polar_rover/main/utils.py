from dataclasses import dataclass

import polar_rover

try:
    import importlib.resources as pkg_resources
except ImportError:
    import importlib_resources as pkg_resources

CSV_ROUTE = pkg_resources.open_text(polar_rover, 'data.csv')
KEY = '6,wD-Ak]^wzWe@G'  # expected key for verification

@dataclass(frozen=True)
class Output():
    """Inmutable class for containing return message and code."""
    message: str
    return_code: int
