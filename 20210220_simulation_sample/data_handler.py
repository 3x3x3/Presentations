# -*- coding: utf-8 -*-
import threading
import time
import global_def as gd
from db_reader import DbReaderDef, DbReaer
from queue import Queue, Empty


class DataHandlerThd(threading.Thread):
    def __init__(self, req_queue: Queue, rcv_queue: Queue, db_host: str, db_port: int, db_user: str, db_pw: str, db_name: str, db_char_set: str = 'utf8'):
        threading.Thread.__init__(self)

        self._db_host = db_host
        self._db_port = db_port
        self._db_user = db_user
        self._db_pw = db_pw
        self._db_name = db_name
        self._db_char_set = db_char_set

        self._req_queue = req_queue
        self._rcv_queue = rcv_queue
        self.is_run = False

    def _send_err_msg(self, msg: str) -> None:
        self._rcv_queue.put({
            gd.KEY_NM_EVT: gd.EVT_TYPE_ERR,
            gd.KEY_NM_MSG: msg
        })

    def _read_db(self, req: dict) -> bool:
        req_date = int(req.get(gd.KEY_NM_DATE, 0))
        tbl_infos = req.get(gd.KEY_NM_TBL_INFOS, None)

        if 19900101 > req_date or 30000101 < req_date:
            self._send_err_msg('Invalid Date')
            return False

        if list != type(tbl_infos) or 0 == len(tbl_infos):
            self._send_err_msg('Invalid Table Infos1')
            return False

        db_readers = []

        for reader_idx, tbl_info in enumerate(tbl_infos):
            tbl_nm = tbl_info.get(gd.KEY_NM_TBL_NM, None)
            col_nms = tbl_info.get(gd.KEY_NM_COL_NMS, [])

            if tbl_nm is None or 0 == len(col_nms):
                self._send_err_msg('Invalid Table Infos2')
                return False

            db_reader = DbReaer(reader_idx, req_date, tbl_nm, col_nms, self._db_host, self._db_port, self._db_user, self._db_pw, self._db_name, self._db_char_set)
            db_readers.append(db_reader)

        for db_reader in db_readers:
            db_reader.read_thd.start()

        is_st_read = False
        is_error = False
        while not is_st_read:
            for db_reader in db_readers:
                thd_state: int = db_reader.get_thd_state()

                if DbReaderDef.STATE_ERROR == thd_state:
                    is_st_read = True
                    is_error = True
                    break
                elif DbReaderDef.STATE_READY == thd_state:
                    break
            else:
                is_st_read = True
            
            time.sleep(0.5)
        
        if is_error:
            for db_reader in db_readers:
                db_reader.set_stop_thd()
            
            time.sleep(1)
            self._send_err_msg('Error in DbReaderThd1')
            return False
        
        # 처음에 하나씩 데이터를 읽는다
        empty_reader_idxs = []
        for reader_idx, db_reader in enumerate(db_readers):
            if not db_reader.read_next_data():
                empty_reader_idxs.append(reader_idx)
        
        # 텅빈 Reader들을 목록에서 제거
        for reader_idx in empty_reader_idxs:
            del db_readers[reader_idx]

        reader_cnt = len(db_readers)
        fin_readers = []

        while 0 < reader_cnt:
            min_rtime_idx = -1
            min_rtime = 9999999999999
            find_min_ts = False
            is_exist_fin_readers = False

            for idx, db_reader in enumerate(db_readers):
                row: list = db_reader.last_data

                # 마지막 데이터가 비었을때
                if row is None:
                    thd_state = db_reader.get_thd_state()

                    if DbReaderDef.STATE_WORKING == thd_state:
                        time.sleep(0.5)
                        db_reader.read_next_data()
                        find_min_ts = False
                        break
                    elif DbReaderDef.STATE_FINISHED == thd_state:
                        fin_readers.append(idx)
                        is_exist_fin_readers = True
                        continue
                    elif DbReaderDef.STATE_ERROR == thd_state:
                        self._send_err_msg('Error in DbReaderThd2')
                        fin_readers.append(idx)
                        is_exist_fin_readers = True
                        continue
                
                pk_rtime = row[0]

                if min_rtime > pk_rtime:
                    min_rtime = pk_rtime
                    min_rtime_idx = idx
                    find_min_ts = True
            
            # 가장 과거의 값을 찾았다면
            if find_min_ts:
                target_reader: DbReaer = db_readers[min_rtime_idx]

                self._rcv_queue.put({
                    gd.KEY_NM_EVT: gd.EVT_TYPE_READ_DB,
                    gd.KEY_NM_IDX: target_reader.reader_idx,
                    gd.KEY_NM_DATA: target_reader.last_data
                })

                target_reader.read_next_data()
            
            # 종료된 Reader가 생겼다면
            if is_exist_fin_readers:
                fin_readers.sort(reverse=True)

                for fin_reader_idx in fin_readers:
                    del db_readers[fin_reader_idx]
                
                reader_cnt = len(db_readers)
                fin_readers.clear()

        self._rcv_queue.put({
            gd.KEY_NM_EVT: gd.EVT_TYPE_FIN
        })

        return True

    def run(self):
        self.is_run = True

        while self.is_run:
            try:
                req = self._req_queue.get(True, 1)
                evt_type = req.get(gd.KEY_NM_EVT)

                if gd.EVT_TYPE_READ_DB == evt_type:
                    print(f'Read DB Start!, data: {req}')
                    self._read_db(req)
                    print(f'Read DB End!, data: {req}')
                elif gd.EVT_TYPE_FIN == evt_type:
                    break
            except Empty as em:
                pass
            except Exception as e:
                self.is_run = False
                break
