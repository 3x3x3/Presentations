import zmq
import time

context = zmq.Context()
socket = context.socket(zmq.REQ)
socket.connect("tcp://localhost:5555")

for i in range(10):
    req_msg = f'Hello World {i}'

    socket.send(req_msg.encode())
    print('req:', req_msg)

    msg = socket.recv()
    print('rcv:', msg)

    time.sleep(1)
