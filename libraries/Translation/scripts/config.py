"""
Config data
"""

import datetime
import os
import platform

from libs import polib


META = "engine/meta"

RECIPES = {
    "ENGINE": [META, "engine/common", "engine/zdoom"],
    "GAMES": [
        "games/chex",
        "games/doom",
        "games/doom2",
        "games/heretic",
        "games/hexen",
        "games/hexen-deathkings",
        "games/plutonia",
        "games/strife",
        "games/tnt"
    ],
    "GAMES_CHEX3": [],
    "GAMES_HARMONY": ["games/harmony"],
    "GAMES_HACX": ["games/hacx"]
}
RECIPES["ALL"] = list(set(sum(RECIPES.values(), [])))

SOURCE_LANG = "en_US"
SOURCE_LANG_ALT = SOURCE_LANG.split("_", maxsplit=1)[0]

# auto-add once a certain portion of strings have been translated
THRESHOLD = 0.5

# add to table  even if THRESHOLD is not met
ENABLED = [
    "en_GB",  # eng enc ena enz eni ens enj enb enl ent enw
    "cs",
    "da",
    "de",
    "es",
    "es_MX",  # esm
    "eo",
    "fi",
    "fr",
    "hu",
    "it",
    "ja",  # jp
    "ko",
    "nl",
    "nb_NO",  # no
    "pl",
    "pt",  # ptg
    "pt_BR",  # pt
    "ro",
    "ru",
    "sr",
    "tr",
]

# Don't add to table even if THRESHOLD is met
# If adding a language here, please add a comment why, so it can be
# re-evaluated later
DISABLED = [
    "ar",  # no rtl support in uzdoom yet
    "he",  # no rtl support in uzdoom yet
]

KEEP_REMARKS = False

SCRIPT_ID = f"python {platform.python_version()} polib {polib.__version__}"
TIMESTAMP = datetime.datetime.now().astimezone().strftime("%Y-%m-%d %H:%M%z")

DEBUG = True
try:
    DEBUG = DEBUG or 'DEBUG_LANGUAGE' in os.environ
except OSError:
    pass
