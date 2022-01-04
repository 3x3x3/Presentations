// main.h
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ws_mng.h"
#include <simdjson.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////

class MainFrm {
public:
    MainFrm();

    void run_forever();

    void request_lob(int ws_id);
    void on_ws_receive(std::string msg);

protected:
    long long get16dTs();

    WsMng m_ws_mng;
    bool m_is_stop;
    simdjson::ondemand::parser m_parser_sj;
};