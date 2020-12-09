#! /usr/bin/env python

import websocket
import time

ws = websocket.create_connection("ws://localhost:9849/websocket_echo")

ws.send("hello")
print(ws.recv())
ws.send("test")
print(ws.recv())
ws.send("hi")
print(ws.recv())

ws.ping("test ping")
ping_echo = ws.recv_frame()
print(ping_echo)

ws.pong("test pong")
pong_echo = ws.recv_frame()
print(pong_echo)

ws.close()

