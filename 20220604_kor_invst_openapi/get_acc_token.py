import requests
import pprint
import common

if '__main__' == __name__:
    app_key, app_secret = common.get_keys('config_dev.ini')

    req_url = f'{common.BASE_DEV_REST_URL}/oauth2/tokenP'

    req_body = {
        'grant_type': 'client_credentials',
        'appkey': app_key,
        'appsecret': app_secret
    }

    resp: dict = requests.post(url=req_url, json=req_body).json()

    pprint.pprint(resp)
