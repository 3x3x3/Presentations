import sys
import zmq
import time
import random


def main(argv):
    in_str = 'hello world'

    if 2 == len(argv):
        in_str = argv[1]

    ctx = zmq.Context()
    socket = ctx.socket(zmq.REQ)
    socket.connect("tcp://127.0.0.1:5557")

    try:
        for i in range(100):
            req_msg = f'{in_str} {i}'
            req_msg = req_msg.encode()

            socket.send(req_msg)
            print('send:', req_msg)

            while True:
                try:
                    msg = socket.recv(zmq.DONTWAIT)
                    print('recv:', msg)
                    break
                except zmq.Again:
                    time.sleep(0.01)

            time.sleep(random.randrange(10, 30) * 0.1)

    except KeyboardInterrupt:
        pass

    socket.close()


if __name__ == '__main__':
    print('start')
    main(sys.argv)
    print('end')
