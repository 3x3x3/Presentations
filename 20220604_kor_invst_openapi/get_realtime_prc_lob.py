import common
import websocket
import json
import threading
import time


def subscribe(ws: websocket.WebSocketApp, tr_id: str, shtcode: str):
    app_key, app_secret = common.get_keys('config_dev.ini')

    req = {
        'header': {
            'appkey': app_key,
            'appsecret': app_secret,
            'custtype': 'P',
            'tr_type': '1',
            'content-type': 'utf-8'
        },
        'body': {
            'input': {
                'tr_id': tr_id,
                'tr_key': shtcode,
            }
        }
    }

    ws.send(json.dumps(req))


def unsubscribe(ws: websocket.WebSocketApp, tr_id: str, shtcode: str):
    app_key, app_secret = common.get_keys('config_dev.ini')

    req = {
        'header': {
            'appkey': app_key,
            'appsecret': app_secret,
            'custtype': 'P',
            'tr_type': '2',
            'content-type': 'utf-8'
        },
        'body': {
            'input': {
                'tr_id': tr_id,
                'tr_key': shtcode,
            }
        }
    }

    ws.send(json.dumps(req))


def wait_close(ws: websocket.WebSocketApp):
    # 30초간 수신
    time.sleep(30)
    unsubscribe(ws, 'H0STCNT0', '005930')  # Current Price
    unsubscribe(ws, 'H0STASP0', '005930')  # Limit Order Book
    time.sleep(1)
    ws.close()


def on_message(ws: websocket.WebSocketApp, msg: str):
    first_str = msg[0]

    # json으로 처리를 해야할 경우
    if '0' != first_str and '1' != first_str:
        rcv: dict = json.loads(msg)
        trid = rcv['header']['tr_id']

        if 'PINGPONG' == trid:
            ws.send(msg)

        print(msg)
        return

    print(msg)


def on_error(ws: websocket.WebSocketApp, error: str):
    print(error)


def on_close(ws: websocket.WebSocketApp, close_status_code, close_msg):
    print('### closed ###')


def on_open(ws: websocket.WebSocketApp):
    print('Opened connection')

    subscribe(ws, 'H0STCNT0', '005930')  # Current Price
    subscribe(ws, 'H0STASP0', '005930')  # Limit Order Book

    # 30초후 종료
    thd = threading.Thread(target=wait_close, args=(ws,), daemon=True)
    thd.start()


if __name__ == '__main__':
    websocket.enableTrace(True)
    websocket = websocket.WebSocketApp(common.BASE_DEV_WS_URL, on_open=on_open, on_message=on_message, on_error=on_error, on_close=on_close)

    websocket.run_forever()
