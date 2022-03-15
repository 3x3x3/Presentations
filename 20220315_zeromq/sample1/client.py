import zmq

context = zmq.Context()
socket = context.socket(zmq.REQ)
socket.connect("tcp://localhost:5555")

req_msg = b'Hello World'

socket.send(req_msg)
print('req:', req_msg)

msg = socket.recv()
print('rcv:', msg)
