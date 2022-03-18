import zmq
import time


def main():
    ctx = zmq.Context()
    socket = ctx.socket(zmq.REQ)
    socket.connect("tcp://127.0.0.1:5555")

    for i in range(10):
        req_msg = f'Hello World {i}'
        req_msg = req_msg.encode()

        socket.send(req_msg)
        print('send:', req_msg)

        msg = socket.recv()
        print('recv:', msg)

        time.sleep(1)

    socket.close()


if __name__ == '__main__':
    print('start')
    main()
    print('end')
