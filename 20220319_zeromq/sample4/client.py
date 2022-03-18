import zmq
import sys
import time


def main(argv):
    in_str = 'hello world'

    if 2 == len(argv):
        in_str = argv[1]

    ctx = zmq.Context()
    socket = ctx.socket(zmq.DEALER)
    socket.connect("tcp://127.0.0.1:5560")

    try:
        send_cnt = 0

        while True:
            send_cnt += 1
            send = f'{in_str} [{send_cnt}]'
            socket.send_string(send)
            print('send:', send)

            for i in range(5):
                while True:
                    try:
                        msg = socket.recv_string(zmq.DONTWAIT)
                        print('rcv:', msg)
                        break
                    except zmq.Again:
                        time.sleep(0.01)

            time.sleep(5)

    except KeyboardInterrupt:
        pass

    socket.close()


if __name__ == '__main__':
    print('start')
    main(sys.argv)
    print('end')
