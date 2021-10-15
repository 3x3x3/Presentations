#define PY_SSIZE_T_CLEAN
#include <Python.h>

int main() {
    Py_Initialize();

    if ( Py_IsInitialized() ) {
        PyRun_SimpleString("print(\'Hello World !!\')\n");
        Py_FinalizeEx();
    }

    return 0;
}
