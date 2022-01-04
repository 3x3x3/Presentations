// main.h
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ws_mng.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////

class MainFrm {
public:
    MainFrm();

    void run_forever();

    void request_lob(int ws_id);
    void on_ws_receive(std::string msg);

protected:
    WsMng m_ws_mng;
    bool m_is_stop;
};