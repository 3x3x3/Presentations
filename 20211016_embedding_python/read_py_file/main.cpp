// py파일을 불러와서 업비트에서 KRW-BTC의 티커 API를 호출

#define PY_SSIZE_T_CLEAN
#include <Python.h>

int main(int argc, char *argv[]) {
    Py_Initialize();

    if ( !Py_IsInitialized() ) {
        return 0;
    }

    // 경로를 등록
    wchar_t* w_argv = Py_DecodeLocale(argv[0], NULL);
    PySys_SetArgv(argc, &w_argv);
    PyMem_RawFree(w_argv);

    // import script.my_module
    PyObject* myModule = PyImport_ImportModule("script.my_module");

    // trd_prc = script.my_module.get_trd_prc()
    PyObject* funcGetTrdPrc = PyUnicode_FromString("get_trd_prc");
    PyObject* trd_prc = PyObject_CallMethodNoArgs(myModule, funcGetTrdPrc);
    Py_DECREF(funcGetTrdPrc);

    // print(trade_price)
    printf("%.8lf\n", PyFloat_AsDouble(trd_prc));

    Py_DECREF(trd_prc);
    Py_DECREF(myModule);

    Py_FinalizeEx();

    return 0;
}
