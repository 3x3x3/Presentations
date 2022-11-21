# -*- coding: utf-8 -*-
import threading
import websocket
import time
import json
from queue import Queue


class BinanceWsThd(threading.Thread):
    def __init__(self, thd_nm: str, base_endpoint: str, req_queue: Queue, rcv_queue: Queue) -> None:
        threading.Thread.__init__(self)

        self.daemon = True
        self.is_run = True
        self._last_ws_close_ts = 0

        self._thd_nm = thd_nm
        self._base_endpoint = base_endpoint
        self._req_queue = req_queue
        self._rcv_queue = rcv_queue

    def _on_open(self, ws) -> None:
        print(f'[{self._thd_nm}] on_open')

    def _on_message(self, ws, rcv: str) -> None:
        pass

    def _on_error(self, ws, msg: str) -> None:
        print(f'[{self._thd_nm}] on_error: {msg}')

    def _on_close(self, ws, status_code, msg: str) -> None:
        print(f'[{self._thd_nm}] on_close')

    def run(self) -> None:
        while self.is_run:
            websocket.enableTrace(False)

            ws_obj = websocket.WebSocketApp(
                self._base_endpoint,
                on_open=self._on_open,
                on_message=self._on_message,
                on_error=self._on_error,
                on_close=self._on_close
            )
            ws_obj.run_forever()

            cur_ts = time.time()

            # 마지막으로 끊어진지 5분도 안되었다면 이상한거다
            if self._last_ws_close_ts + 300 > cur_ts:
                print(f'[{self._thd_nm}] Cannot reconnect binance ws server !!')
                break

            self._last_ws_close_ts = cur_ts


class BinanceFutWsThd(BinanceWsThd):
    THD_NAME = "fut_thd"
    BASE_ENDPOINT = "wss://fstream.binance.com"

    def __init__(self, req_queue: Queue, rcv_queue: Queue) -> None:
        super(BinanceFutWsThd, self).__init__(
            BinanceFutWsThd.THD_NAME,
            BinanceFutWsThd.BASE_ENDPOINT,
            req_queue,
            rcv_queue
        )

    def _on_message(self, ws, rcv: str) -> None:
        data = json.loads(rcv)


class BinanceOptWsThd(BinanceWsThd):
    THD_NAME = "opt_thd"
    BASE_ENDPOINT = "wss://nbstream.binance.com/eoptions/"

    def __init__(self, req_queue: Queue, rcv_queue: Queue) -> None:
        super(BinanceOptWsThd, self).__init__(
            BinanceOptWsThd.THD_NAME,
            BinanceOptWsThd.BASE_ENDPOINT,
            req_queue,
            rcv_queue
        )

    def _on_message(self, ws, rcv: str) -> None:
        data = json.loads(rcv)
