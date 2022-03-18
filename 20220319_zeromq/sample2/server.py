import zmq
import time
import random


def main():
    ctx = zmq.Context.instance()
    socket = ctx.socket(zmq.PUB)
    socket.bind("tcp://*:5556")

    try:
        while True:
            topic = f'{random.randrange(0, 5)}'
            msg = f'{random.randrange(0, 100)}'

            tot_msg = f'{topic}-{msg}'

            socket.send_string(tot_msg)
            #socket.send_multipart([topic.encode(), msg.encode()])

            print('send:', tot_msg)

            time.sleep(0.5)

    except KeyboardInterrupt:
        pass

    socket.close()
    ctx.term()


if __name__ == '__main__':
    print('start')
    main()
    print('end')
