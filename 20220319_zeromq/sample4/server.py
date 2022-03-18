import time

import zmq


def main():
    ctx = zmq.Context()
    socket = ctx.socket(zmq.ROUTER)
    socket.bind("tcp://*:5560")

    try:
        while True:
            while True:
                try:
                    rcv = socket.recv_multipart(zmq.DONTWAIT)
                    print('recv:', rcv)
                    break
                except zmq.Again:
                    time.sleep(0.01)

            for i in range(5):
                send = [rcv[0], rcv[1]]
                socket.send_multipart(send)
                print('send:', send)

                time.sleep(0.2)

    except KeyboardInterrupt:
        pass

    socket.close()
    ctx.term()


if __name__ == '__main__':
    print('start')
    main()
    print('end')
