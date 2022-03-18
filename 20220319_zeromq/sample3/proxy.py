import zmq
import threading
import time


def main():
    ctx = zmq.Context()
    front = ctx.socket(zmq.ROUTER)
    front.bind('tcp://127.0.0.1:5557')
    back = ctx.socket(zmq.DEALER)
    back.bind('tcp://127.0.0.1:5558')

    try:
        thd = threading.Thread(target=zmq.proxy, args=(front, back, ), daemon=True)
        thd.start()

        while True:
            time.sleep(0.1)

    except KeyboardInterrupt:
        pass

    front.close()
    back.close()
    ctx.term()


if __name__ == '__main__':
    print('start')
    main()
    print('end')
