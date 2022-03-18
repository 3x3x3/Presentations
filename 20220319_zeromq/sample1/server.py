import zmq


def main():
    ctx = zmq.Context()
    socket = ctx.socket(zmq.REP)
    socket.bind('tcp://*:5555')

    while True:
        msg = socket.recv()
        print('recv:', msg)

        socket.send(msg)
        print('send:', msg)


if __name__ == '__main__':
    print('start')
    main()
    print('end')
