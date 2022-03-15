import zmq
import threading


def run(sub_filter: str):
    ctx = zmq.Context()
    sock = ctx.socket(zmq.SUB)
    sock.connect(f'tcp://localhost:5556')
    sock.setsockopt(zmq.SUBSCRIBE, sub_filter.encode())

    while True:
        msg = sock.recv()
        print(f'sub_filter: {sub_filter}, rcv: {msg}')


if __name__ == '__main__':
    thds = list()

    for i in range(5):
        thd = threading.Thread(target=run, args=(f'{i}',))
        thd.start()
        thds.append(thd)

    for thd in thds:
        thd.join()
