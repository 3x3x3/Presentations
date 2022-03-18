import sys
import zmq
import time


def main(argv):
    channel = ''

    if 2 == len(argv):
        channel = argv[1]

    ctx = zmq.Context.instance()
    sock = ctx.socket(zmq.SUB)
    sock.connect(f'tcp://127.0.0.1:5556')
    sock.set(zmq.SUBSCRIBE, channel.encode())

    while True:
        try:
            msg = sock.recv(zmq.DONTWAIT)
            print(f'channel: {channel}, rcv: {msg}')
        except zmq.Again:
            time.sleep(0.01)


if __name__ == '__main__':
    main(sys.argv)
