#!/usr/bin/env python

'''
Sets up websocket server support to run the server in one HTML page and the client in another HTML page. Each connects to a websocket server, which we relay together, so the two pages think they are connected to each other (see websocket_bi tests in emscripten).

Instructions for websocket networking:

Mode 1: Two clients (one with embedded server)

  1. Run this script
  2. Run a webserver (e.g. python -m SimpleHTTPServer 8888)
  3. Run http://localhost:8888/game.html?low,low,windowed,serve in one browser
  4. Run http://localhost:8888/game.html?low,low,windowed in another browser
  5. In the second browser, do /connect

  'windowed' runs in non-fullscreen mode, useful to run two browsers at once - scroll
  all the way down to see the canvas. 'serve' runs the embedded server in that
  client.

Mode 2: Server and client

  1. Run this script
  2. Run a webserver (e.g. python -m SimpleHTTPServer 8888)
  3. Run http://localhost:8888/server.html
  4. Run http://localhost:8888/game.html?low,low
  5. In the client, do /connect

Note that you likely need to run the server and client in different browsers or at least browser windows, since browsers throttle background tabs.
'''

import os, sys, multiprocessing, time
from subprocess import Popen, PIPE, STDOUT

__rootpath__ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)

sys.path += [path_from_root('tools/'), path_from_root('tools/websockify')]

import websockify

def websockify_func(wsp):
  wsp.start_server()

client = websockify.WebSocketProxy(verbose=True, listen_port=28785, target_host="127.0.0.1", target_port=28786, run_once=True)
client_process = multiprocessing.Process(target=websockify_func, args=(client,))
client_process.start()
print 'client on process', client_process.pid

server = websockify.WebSocketProxy(verbose=True, listen_port=28780, target_host="127.0.0.1", target_port=28781, run_once=True)
server_process = multiprocessing.Process(target=websockify_func, args=(server,))
server_process.start()
print 'server on process', server_process.pid

def relay_server(child):
  child.communicate()

relay_child = Popen(['python', path_from_root('tools', 'socket_relay.py'), '28781', '28786'])
relay_process = multiprocessing.Process(target=relay_server, args=(relay_child,))
relay_process.start()
print 'relay on process', relay_process.pid

while 1:
  time.sleep(1)

