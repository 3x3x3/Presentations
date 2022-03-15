import zmq

context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://*:5555")

while True:
    msg = socket.recv()
    print('rcv:', msg)

    socket.send(msg)
    print('req:', msg)
