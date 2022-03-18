import sys
import zmq
import time


def main(argv):
    topic = ''

    if 2 == len(argv):
        topic = argv[1]

    ctx = zmq.Context.instance()
    socket = ctx.socket(zmq.SUB)
    socket.connect(f'tcp://127.0.0.1:5556')
    socket.set(zmq.SUBSCRIBE, topic.encode())

    try:
        while True:
            try:
                msg = socket.recv(zmq.DONTWAIT)
                #msg = socket.recv_multipart(zmq.DONTWAIT)

                print(f'input topic: {topic}, rcv: {msg}')
            except zmq.Again:
                time.sleep(0.01)
    except KeyboardInterrupt:
        pass

    socket.close()
    ctx.term()


if __name__ == '__main__':
    print('start')
    main(sys.argv)
    print('end')
