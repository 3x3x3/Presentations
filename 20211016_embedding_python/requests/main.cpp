// 파이썬의 requests를 이용하여 업비트에서 KRW-BTC의 티커 API를 호출

#define PY_SSIZE_T_CLEAN
#include <Python.h>

int main() {
    /*
    다음을 구현
    import requests
    resp = requests.get("https://api.upbit.com/v1/ticker?markets=KRW-BTC")
    text = resp.text
    print(text)

    dict_obj = resp.json()
    trade_price = dict_obj[0]["trade_price"]
    print(trade_price)
    */

    Py_Initialize();

    if ( !Py_IsInitialized() ) {
        return 0;
    }

    // import requests
    PyObject* reqModule = PyImport_ImportModule("requests");

    // resp = requests.get("https://api.upbit.com/v1/ticker?markets=KRW-BTC")
    PyObject* resp = PyObject_CallMethod(reqModule, "get", "s", "https://api.upbit.com/v1/ticker?markets=KRW-BTC");

    // text = resp.text
    PyObject* text = PyObject_GetAttrString(resp, "text");
    
    // print(text)
    printf("%s\n", PyUnicode_AsUTF8(text));
    Py_DECREF(text);

    // dict_obj = resp.json()
    PyObject* funcJson = PyUnicode_FromString("json");
    PyObject* dict_obj = PyObject_CallMethodNoArgs(resp, funcJson);
    Py_DECREF(funcJson);

    // trade_price = dict_obj[0]["trade_price"]
    PyObject* dictObjZero = PyList_GetItem(dict_obj, 0);
    PyObject* trade_price = PyDict_GetItemString(dictObjZero, "trade_price");

    // print(trade_price)
    printf("%.8lf\n", PyFloat_AsDouble(trade_price));

    Py_DECREF(dict_obj);
    Py_DECREF(resp);
    Py_DECREF(reqModule);

    Py_FinalizeEx();

    return 0;
}
