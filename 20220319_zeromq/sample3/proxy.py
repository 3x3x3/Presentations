import zmq
import threading
import time


def main():
    ctx = zmq.Context()
    front = ctx.socket(zmq.ROUTER)
    front.bind('tcp://127.0.0.1:5557')
    back = ctx.socket(zmq.DEALER)
    back.bind('tcp://127.0.0.1:5558')

    zmq.proxy(front, back)


if __name__ == '__main__':
    print('start')

    try:
        thd = threading.Thread(target=main, daemon=True)
        thd.start()

        while True:
            time.sleep(0.1)

    except KeyboardInterrupt:
        pass

    print('end')
