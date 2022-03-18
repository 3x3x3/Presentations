import sys
import zmq
import time


def main(argv):
    channel = ''

    if 2 == len(argv):
        channel = argv[1]

    ctx = zmq.Context.instance()
    socket = ctx.socket(zmq.SUB)
    socket.connect(f'tcp://127.0.0.1:5556')
    socket.set(zmq.SUBSCRIBE, channel.encode())

    try:
        while True:
            try:
                msg = socket.recv(zmq.DONTWAIT)
                print(f'channel: {channel}, rcv: {msg}')
            except zmq.Again:
                time.sleep(0.01)
    except KeyboardInterrupt:
        pass

    socket.close()


if __name__ == '__main__':
    print('start')
    main(sys.argv)
    print('end')
