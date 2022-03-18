import zmq
import time
import random


def main():
    ctx = zmq.Context.instance()
    socket = ctx.socket(zmq.PUB)
    socket.bind("tcp://*:5556")

    try:
        while True:
            msg = f'{random.randrange(0, 5)}-{random.randrange(0, 100)}'
            socket.send(msg.encode())
            print('send:', msg)

            time.sleep(0.5)

    except KeyboardInterrupt:
        pass

    socket.close()


if __name__ == '__main__':
    print('start')
    main()
    print('end')
