import zmq
import time


def main():
    context = zmq.Context()
    socket = context.socket(zmq.REQ)
    socket.connect("tcp://localhost:5555")

    for i in range(10):
        req_msg = f'Hello World {i}'
        req_msg = req_msg.encode()

        socket.send(req_msg)
        print('req:', req_msg)

        msg = socket.recv()
        print('rcv:', msg)

        time.sleep(1)


if __name__ == '__main__':
    main()
