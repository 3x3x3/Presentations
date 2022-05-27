import requests
import pprint
import common

if '__main__' == __name__:
    app_key, app_secret = common.get_keys('config.ini')

    req_url = f'{common.BASE_REST_URL}/uapi/hashkey'

    req_header = {
        'content-type': 'application/json; charset=utf-8',
        'appkey': app_key,
        'appsecret': app_secret,
    }

    data = {}  # insert my data

    resp: dict = requests.post(url=req_url, headers=req_header, json=data).json()

    pprint.pprint(resp)
