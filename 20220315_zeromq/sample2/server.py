import zmq
import time
import random


def main():
    context = zmq.Context()
    socket = context.socket(zmq.PUB)
    socket.bind("tcp://*:5556")

    while True:
        msg = f'{random.randrange(0, 5)}-{random.randrange(0, 100)}'
        socket.send(msg.encode())
        print('send:', msg)

        time.sleep(0.5)


if __name__ == '__main__':
    main()
