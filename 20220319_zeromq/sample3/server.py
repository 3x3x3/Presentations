import zmq
import time


def main():
    ctx = zmq.Context()
    socket = ctx.socket(zmq.REP)
    socket.connect('tcp://127.0.0.1:5558')

    try:
        while True:
            while True:
                try:
                    msg = socket.recv(zmq.DONTWAIT)
                    print('recv:', msg)
                    break
                except zmq.Again:
                    time.sleep(0.01)

            socket.send(msg)
            print('send:', msg)
    except KeyboardInterrupt:
        pass

    socket.close()
    ctx.term()


if __name__ == '__main__':
    print('start')

    try:
        main()
    except KeyboardInterrupt:
        pass

    print('end')
