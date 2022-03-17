import sys
import zmq


def main(argv):
    channel = ''

    if 2 == len(argv):
        channel = argv[1]

    ctx = zmq.Context.instance()
    sock = ctx.socket(zmq.SUB)
    sock.connect(f'tcp://localhost:5556')
    sock.set(zmq.SUBSCRIBE, channel.encode())

    while True:
        msg = sock.recv()
        print(f'channel: {channel}, rcv: {msg}')


if __name__ == '__main__':
    main(sys.argv)
