# -*- coding: utf-8 -*-
import threading
import socket
import socketserver
import configparser
import struct
import msgpack
import datetime
from decimal import Decimal
from queue import Queue, Empty
from data_handler import DataHandlerThd


class ThreadedTCPRequestHandler(socketserver.BaseRequestHandler):
    def __init__(self, request, client_address, server):
        super().__init__(request, client_address, server)
        self.is_run = False

    def _request_listener(self, req_queue: Queue):
        while self.is_run:
            try:
                header = self.request.recv(4, socket.MSG_WAITALL)

                if header:
                    body_len = struct.unpack('I', header)[0]
                    body_len = socket.ntohl(body_len)
                    body = self.request.recv(body_len, socket.MSG_WAITALL)
                    req_data = msgpack.unpackb(body, raw=False)

                    req_queue.put(req_data)
                else:
                    break

            except Exception as e:
                self.is_run = False
                break

    @classmethod
    def _pack_default(cls, val: any):
        if isinstance(val, datetime.date):
            return str(val)
        elif isinstance(val, Decimal):
            return float(val)
        raise TypeError('Exception in msg_pack_default')

    def _receive_listener(self, rcv_queue: Queue):
        while self.is_run:
            try:
                rcv_data = rcv_queue.get(True, 1)

                body = msgpack.packb(rcv_data, use_bin_type=True, default=self._pack_default)
                header = struct.pack('I', socket.htonl(len(body)))

                # 내용의길이(4byte, int) + 내용(bytes)
                self.request.sendall(header + body)

            except Empty as em:
                pass
            except Exception as e:
                self.is_run = False
                break

    def handle(self):
        self.is_run = True

        req_queue = Queue()
        rcv_queue = Queue()

        config = configparser.ConfigParser()
        config.read('config.ini')

        db_host = config.get('DB', 'HOST')
        db_port = int(config.get('DB', 'PORT'))
        db_user = config.get('DB', 'USER')
        db_pw = config.get('DB', 'PASSWORD')
        db_name = config.get('DB', 'DB_NAME')
        db_charset = config.get('DB', 'CHAR_SET')

        data_handler_thd = DataHandlerThd(req_queue, rcv_queue, db_host, db_port, db_user, db_pw, db_name, db_charset)
        data_handler_thd.start()

        req_thd = threading.Thread(target=self._request_listener, args=(req_queue,))
        req_thd.start()

        rcv_thd = threading.Thread(target=self._receive_listener, args=(rcv_queue,))
        rcv_thd.start()

        req_thd.join()
        rcv_thd.join()

        data_handler_thd.is_run = False


class ThreadedTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    pass


if __name__ == '__main__':
    HOST = ''
    PORT = 8765

    svr = ThreadedTCPServer((HOST, PORT), ThreadedTCPRequestHandler)

    with svr:
        ip, port = svr.server_address

        server_thread = threading.Thread(target=svr.serve_forever)
        server_thread.daemon = True
        server_thread.start()
        server_thread.join()

        svr.shutdown()
