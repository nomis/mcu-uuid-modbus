# -*- coding: utf-8 -*-

import os

rtd = os.environ.get("READTHEDOCS") == 'True'

needs_sphinx = '1.3'
extensions = []
source_suffix = ['.rst']

project = u'mcu-uuid-modbus'
copyright = u'2021-2024, Simon Arlott'
author = u'Simon Arlott'

master_doc = 'index'

version = u''
release = u''

language = 'en'
exclude_patterns = ['build', 'Thumbs.db', '.DS_Store']
pygments_style = 'sphinx'
highlight_language = 'c++'
todo_include_todos = False

if rtd:
	html_theme = 'sphinx_rtd_theme'

linkcheck_timeout = 60
linkcheck_ignore = [r'https://github.com/.+/.+/(compare|commits)/.+']
