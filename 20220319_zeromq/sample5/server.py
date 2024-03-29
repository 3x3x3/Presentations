import zmq
import websocket
import threading
import json
import time
import queue
import uuid

is_run = True
ws_req_queue = queue.Queue()
ws_rcv_queue = queue.Queue()
subscribers = dict()
sub_symbols = dict()


def ws_open(ws):
    print('websocket opened')

    req_thd = threading.Thread(target=ws_req_handler, args=(ws,), daemon=True)
    req_thd.start()


def ws_msg(ws, rcv):
    global ws_rcv_queue
    ws_rcv_queue.put(rcv)
    print('rcv from upbit:', rcv)


def ws_err(ws, rcv):
    pass


def ws_close(ws):
    print('websocket closed')


def ws_req_handler(ws):
    global ws_req_queue

    while True:
        try:
            req_symbols = ws_req_queue.get_nowait()

            send_dict = [{
                'ticket': str(uuid.uuid4())
            }, {
                'type': 'trade',
                'codes': req_symbols
            }, {
                'format': 'SIMPLE'
            }]

            send_json = json.dumps(send_dict)
            ws.send(send_json)

            print('send to upbit:', send_dict)

        except queue.Empty:
            time.sleep(1)


def zmq_heartbeat_handler(socket):
    global subscribers
    global sub_symbols

    send_bin = json.dumps({'ty': 'ping'}).encode()
    del_addrs = list()

    while True:
        cur_ts = time.time()

        for addr, sub_info in subscribers.items():
            hb_ts = sub_info['hb_ts']

            if hb_ts + 15 < cur_ts:
                symbols = sub_info['symbols']
                for symbol in symbols:
                    sub_symbols[symbol].remove(addr)

                del_addrs.append(addr)

                continue

            send = [addr, send_bin]
            socket.send_multipart(send)

            print('send to client:', send)

        if 0 < len(del_addrs):
            for addr in del_addrs:
                del (subscribers[addr])

            print('disconnected:', del_addrs)
            del_addrs.clear()

        time.sleep(5)


def zmq_rcv_handler(socket):
    global ws_req_queue
    global subscribers
    global sub_symbols

    while True:
        try:
            addr, rcv = socket.recv_multipart(zmq.DONTWAIT)
            print('rcv from client:', rcv)

            rcv_dict = json.loads(rcv.decode())
            ty = rcv_dict.get('ty')

            if 'pong' == ty:
                subscriber = subscribers.get(addr)
                if subscriber is not None:
                    subscriber['hb_ts'] = time.time()

            elif 'subscribe' == ty:
                if addr not in subscribers:
                    subscribers[addr] = {
                        'hb_ts': time.time(),
                        'symbols': set()
                    }

                symbols = rcv_dict.get('symbols', [])

                subscribers[addr]['symbols'].update(symbols)

                for symbol in symbols:
                    if symbol not in sub_symbols:
                        sub_symbols[symbol] = set()

                    sub_symbols[symbol].add(addr)

                req_symbols = list()
                for symbol in sub_symbols:
                    req_symbols.append(symbol)

                ws_req_queue.put(req_symbols)

        except zmq.Again:
            time.sleep(0.01)


def zmq_send_handler(socket):
    global ws_rcv_queue
    global sub_symbols

    while True:
        try:
            rcv_bin: bytes = ws_rcv_queue.get_nowait()
            rcv_dict = json.loads(rcv_bin.decode())

            symbol = rcv_dict.get('cd')
            addrs = sub_symbols.get(symbol, [])

            for addr in addrs:
                send = [addr, rcv_bin]
                socket.send_multipart(send)

        except queue.Empty:
            time.sleep(0.01)


def zmq_handler():
    global is_run

    ctx = zmq.Context()
    socket = ctx.socket(zmq.ROUTER)
    socket.bind('tcp://*:5570')

    zmq_rcv_thd = threading.Thread(target=zmq_rcv_handler, args=(socket,), daemon=True)
    zmq_rcv_thd.start()

    zmq_send_thd = threading.Thread(target=zmq_send_handler, args=(socket,), daemon=True)
    zmq_send_thd.start()

    zmq_heartbeat_thd = threading.Thread(target=zmq_heartbeat_handler, args=(socket,), daemon=True)
    zmq_heartbeat_thd.start()

    while is_run:
        time.sleep(0.5)

    socket.close()
    ctx.term()


def main():
    #websocket.enableTrace(True)
    ws_url = 'wss://api.upbit.com/websocket/v1'
    ws_obj = websocket.WebSocketApp(ws_url, on_open=ws_open, on_message=ws_msg, on_error=ws_err, on_close=ws_close)
    ws_thd = threading.Thread(target=ws_obj.run_forever)
    ws_thd.start()

    zmq_thd = threading.Thread(target=zmq_handler, daemon=True)
    zmq_thd.start()

    try:
        while True:
            time.sleep(0.01)

    except KeyboardInterrupt:
        pass

    global is_run
    is_run = False

    ws_obj.close()
    ws_thd.join()
    zmq_thd.join()


if __name__ == '__main__':
    print('start')
    main()
    print('end')
