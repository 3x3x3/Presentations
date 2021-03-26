# -*- coding: utf-8 -*-
import socket
import msgpack
import struct
import global_def as gd
import time

HOST = '127.0.0.1'
PORT = 8765


def main():
    print('시작')
    
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))

        req_data = {
            gd.KEY_NM_EVT: gd.EVT_TYPE_READ_DB,
            gd.KEY_NM_DATE: 20210113,
            gd.KEY_NM_TBL_INFOS: [
                {
                    gd.KEY_NM_TBL_NM: 'K200MN_FUT_EXEC',
                    gd.KEY_NM_COL_NMS: ['ISIN_CODE', 'PK_RTIME', 'CUR_PRC', 'EXE_Q']
                },
                {
                    gd.KEY_NM_TBL_NM: 'K200MN_FUT_EXEC_LIMIT_ORD',
                    gd.KEY_NM_COL_NMS: ['ISIN_CODE', 'PK_RTIME', 'L_LVL_1_PRC', 'L_LVL_1_Q', 'L_LVL_2_PRC', 'L_LVL_2_Q', 'L_LVL_3_PRC', 'L_LVL_3_Q', 'S_LVL_1_PRC', 'S_LVL_1_Q', 'S_LVL_2_PRC', 'S_LVL_2_Q', 'S_LVL_3_PRC', 'S_LVL_3_Q']
                },
                {
                    gd.KEY_NM_TBL_NM: 'K200_FUT_EXEC',
                    gd.KEY_NM_COL_NMS: ['ISIN_CODE', 'PK_RTIME', 'CUR_PRC', 'EXE_Q']
                },
                {
                    gd.KEY_NM_TBL_NM: 'K200_FUT_EXEC_LIMIT_ORD',
                    gd.KEY_NM_COL_NMS: ['ISIN_CODE', 'PK_RTIME', 'L_LVL_1_PRC', 'L_LVL_1_Q', 'L_LVL_2_PRC', 'L_LVL_2_Q', 'L_LVL_3_PRC', 'L_LVL_3_Q', 'S_LVL_1_PRC', 'S_LVL_1_Q', 'S_LVL_2_PRC', 'S_LVL_2_Q', 'S_LVL_3_PRC', 'S_LVL_3_Q']
                }
            ]
        }

        body = msgpack.packb(req_data, use_bin_type=True)
        header = struct.pack('I', socket.htonl(len(body)))

        # 내용의길이(4byte, int) + 내용(bytes)
        s.sendall(header + body)

        st_time = time.time()

        while True:
            # 수신할 데이터의 길이
            rcv = s.recv(4, socket.MSG_WAITALL)

            if rcv:
                # 수신할 데이터의 길이를 int로 변환
                body_len = struct.unpack('I', rcv)[0]
                body_len = socket.ntohl(body_len)

                # 데이터의 길이만큼 읽음
                body = s.recv(body_len, socket.MSG_WAITALL)

                if body:
                    rcv_dict = msgpack.unpackb(body, raw=False)

                    evt_type = rcv_dict.get(gd.KEY_NM_EVT)

                    if gd.EVT_TYPE_READ_DB == evt_type:
                        print(rcv_dict)
                        #time.sleep(0.01)
                        #pass
                    elif gd.EVT_TYPE_ERR == evt_type:
                        print('에러발생', rcv_dict)
                    elif gd.EVT_TYPE_FIN == evt_type:
                        print(f'수신완료. 소요시간 {time.time() - st_time}s', rcv_dict)
                        break
                    else:
                        break

                else:
                    break

            else:
                break

    print('완료')


if '__main__' == __name__:
    main()
