# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html^

import subprocess
from datetime import datetime, timezone
from importlib import metadata
from pathlib import Path

subprocess.check_call("doxygen", cwd=Path(__file__).parent)

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "openae"
copyright = f"{datetime.now(tz=timezone.utc).date().year}, OpenAE"
author = "OpenAE contributors"
release = metadata.version("openae")

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.viewcode",
    "sphinx.ext.intersphinx",
    "sphinx.ext.napoleon",
    "sphinx.ext.autosummary",
    "sphinx_autodoc_typehints",
    "sphinx_design",
    "myst_parser",
    "breathe",
]

source_suffix = [".rst", ".md"]

intersphinx_mapping = {
    "python": ("https://docs.python.org/3/", None),
    "numpy": ("https://docs.scipy.org/doc/numpy/", None),
}

autodoc_member_order = "bysource"
autodoc_docstring_signature = False
autodoc_default_options = {"members": True, "undoc-members": True}
autodoc_typehints = "both"
autodoc_typehints_description_target = "all"

autosummary_generate = True

myst_enable_extensions = ["colon_fence"]

breathe_projects = {"openae": "_build/doxygen/xml"}
breathe_default_project = "openae"
breathe_default_members = ("members", "undoc-members")
breathe_show_include = False

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "furo"
html_theme_options = {
    "light_css_variables": {
        "color-background-primary": "#ffffff",
        "color-background-secondary": "#f6f6f7",
    },
    "dark_css_variables": {
        "color-background-primary": "#1b1b1f",
        "color-background-secondary": "#161618",
    },
}
html_favicon = "_static/favicon.ico"
html_logo = "_static/icon.png"
html_title = "OpenAE Library"
html_static_path = ["_static"]
