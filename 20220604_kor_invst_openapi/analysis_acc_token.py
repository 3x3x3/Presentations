import jwt
import base64
import json


def encode_jwt(payload: dict, secret_key: str, algorithm: str):
    return jwt.encode(payload, secret_key, algorithm=algorithm)


def decode_base64(val: str) -> str:
    return base64.b64decode(val).decode('utf-8')


if '__main__' == __name__:
    pass
