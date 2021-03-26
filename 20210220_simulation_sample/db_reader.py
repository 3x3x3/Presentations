# -*- coding: utf-8 -*-
import pymysql
import threading
import time
from queue import Queue, Empty


class DbReaderDef:
    STATE_ERROR = -1
    STATE_READY = 0
    STATE_WORKING = 1
    STATE_FINISHED = 2


class DbReaderThd(threading.Thread):
    CHUNK_SIZE = 10000

    def __init__(self, rcv_queue: Queue, req_date: int, tbl_nm: str, col_nms: list, db_host: str, db_port: int, db_user: str, db_pw: str, db_name: str, db_char_set: str = 'utf8'):
        threading.Thread.__init__(self)

        self._rcv_queue = rcv_queue

        self._req_date = req_date
        self._tbl_nm: str = tbl_nm
        self._col_nms_str: str = ', '.join(col_nms)

        if 0 < len(self._col_nms_str):
            self._col_nms_str = f', {self._col_nms_str}'

        self._db_host = db_host
        self._db_port = db_port
        self._db_user = db_user
        self._db_pw = db_pw
        self._db_name = db_name
        self._db_char_set = db_char_set

        self._state = DbReaderDef.STATE_READY
        self._state_lock = threading.Lock()

        self._row_cnt: int = 0
        self._maximum_seq: int = 0
        self._seq_offset: int = 0
        self._cur_pos = 0

        self.is_run = False
        self._db_conn = None

    def __del__(self):
        self._db_close()

    def _db_connect(self):
        try:
            self._db_conn = pymysql.connect(
                host=self._db_host,
                port=self._db_port,
                user=self._db_user,
                passwd=self._db_pw,
                db=self._db_name,
                charset=self._db_char_set,
                cursorclass=pymysql.cursors.Cursor
            )

        except Exception as e:
            self._db_conn = None
            print('DB Connect Failed ({0})'.format(str(e)))
            raise Exception

    def _db_close(self):
        if self._db_conn is not None:
            self._db_conn.close()
            self._db_conn = None

    def set_state(self, state: int) -> None:
        self._state_lock.acquire()
        self._state = state
        self._state_lock.release()

    def get_state(self) -> int:
        self._state_lock.acquire()
        ret_state = self._state
        self._state_lock.release()

        return ret_state

    def run(self):
        self.is_run = True

        try:
            self._db_connect()

            with self._db_conn.cursor() as curs:
                query = f'SELECT COUNT(*) AS cnt, MAX(SEQ) AS max_seq FROM {self._tbl_nm} WHERE YMD=%s;'
                curs.execute(query, (str(self._req_date),))
                rs = curs.fetchone()
                self._row_cnt = rs[0]
                self._maximum_seq = rs[1]
                print(f'tbl_nm: {self._tbl_nm}, YMD: {self._req_date}, row_cnt: {self._row_cnt}')

            while self.is_run and self._seq_offset <= self._row_cnt:
                with self._db_conn.cursor() as curs:
                    query = f'SELECT PK_RTIME, SEQ {self._col_nms_str} FROM {self._tbl_nm} WHERE YMD=%s ORDER BY SEQ LIMIT %s, %s;'
                    curs.execute(query, (str(self._req_date), self._seq_offset, DbReaderThd.CHUNK_SIZE, ))
                    rs = curs.fetchall()

                    self._seq_offset = self._seq_offset + DbReaderThd.CHUNK_SIZE

                    for row in rs:
                        row = list(row)
                        row[0] = float(row[0].replace(':', ''))
                        self._rcv_queue.put(row)

                print(f'tbl_nm: {self._tbl_nm}, YMD: {self._req_date}, seq_offset: {self._seq_offset}')
                self.set_state(DbReaderDef.STATE_WORKING)
                time.sleep(0.1)

        except Exception as e:
            print(e)
            self.set_state(DbReaderDef.STATE_ERROR)
            self._db_close()
            raise Exception

        self._db_close()
        self.set_state(DbReaderDef.STATE_FINISHED)


class DbReaer:
    def __init__(self, reader_idx: int, req_date: int, tbl_nm: str, col_nms: list, db_host: str, db_port: int, db_user: str, db_pw: str, db_name: str, db_char_set: str = 'utf8'):
        self.reader_idx = reader_idx
        self.rcv_queue: Queue = Queue()
        self.read_thd: DbReaderThd = DbReaderThd(self.rcv_queue, req_date, tbl_nm, col_nms, db_host, db_port, db_user, db_pw, db_name, db_char_set)
        self.read_thd.daemon = True

        self.last_data = None

    def get_thd_state(self):
        return self.read_thd.get_state()

    def set_stop_thd(self):
        self.read_thd.is_run = False
    
    def read_next_data(self) -> bool:
        ret = False

        try:
            self.last_data = self.rcv_queue.get_nowait()
            ret = True
        except Empty as em:
            self.last_data = None
            ret = False

        return ret
