#!/usr/bin/env python

import os, sys

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)

sys.path += [path_from_root('tools/'), path_from_root('tools/websockify')]

import websockify

