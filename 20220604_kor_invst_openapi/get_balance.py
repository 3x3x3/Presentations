import common
import requests
import pprint


def main() -> None:
    app_key, app_secret = common.get_keys('config_real.ini')
    _, acc_no, acc_cd = common.get_account_infos('config_real.ini')
    token_type, acc_token = common.get_acc_token(app_key, app_secret, 'acc_token_real.json')

    req_header = {
        'content-type': 'application/json; charset=utf-8',
        'authorization': f'{token_type} {acc_token}',
        'appkey': app_key,
        'appsecret': app_secret,
        'tr_id': 'TTTC8434R',
    }

    req_body = {
        'CANO': acc_no,
        'ACNT_PRDT_CD': acc_cd,
        'AFHR_FLPR_YN': 'N',
        'OFL_YN': '',
        'INQR_DVSN': '02',
        'UNPR_DVSN': '01',
        'FUND_STTL_ICLD_YN': 'N',
        'FNCG_AMT_AUTO_RDPT_YN': 'N',
        'PRCS_DVSN': '00',
        'CTX_AREA_FK100': '',
        'CTX_AREA_NK100': '',
    }

    path = '/uapi/domestic-stock/v1/trading/inquire-balance'
    req_url = f'{common.BASE_REAL_REST_URL}{path}'

    resp = requests.get(req_url, headers=req_header, params=req_body).json()
    pprint.pprint(resp)


if '__main__' == __name__:
    main()
