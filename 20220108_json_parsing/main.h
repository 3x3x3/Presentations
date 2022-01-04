// main.h
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include <simdjson.h>
#include <rapidjson/document.h>
#include "ws_mng.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////

class MainFrm {
public:
    MainFrm();

    void run_forever();

    void request_lob(int ws_id);
    void on_ws_receive(std::string msg);

protected:
    long long get_16d_ts();

    WsMng m_ws_mng;
    bool m_is_stop;

    rapidjson::Document m_doc_rj;
    simdjson::ondemand::parser m_parser_sj;
};