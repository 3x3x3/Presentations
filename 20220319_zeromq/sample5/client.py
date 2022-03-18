import zmq
import sys
import time
import json


def main(argv):
    if 2 > len(argv):
        print('empty params')
        return

    ctx = zmq.Context()
    socket = ctx.socket(zmq.DEALER)
    socket.connect("tcp://127.0.0.1:5570")

    try:
        send_dict = {
            'task': 'subscribe',
            'symbols': argv[1:]
        }

        socket.send_json(send_dict)
        print('send:', send_dict)

        while True:
            while True:
                try:
                    rcv_dict = socket.recv_json(zmq.DONTWAIT)
                    break
                except zmq.Again:
                    time.sleep(0.01)

            print('rcv:', rcv_dict)

            ty = rcv_dict.get('ty')

            # heartbeat
            if 'ping' == ty:
                send_dict = {'task': 'pong'}
                socket.send_json(send_dict)
                print('send:', send_dict)

    except KeyboardInterrupt:
        pass

    socket.close()
    ctx.term()


if __name__ == '__main__':
    print('start')
    main(sys.argv)
    print('end')
