// main.cpp
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <unistd.h>
#include <string>
#include <functional>
#include <chrono>
#include <rapidjson/document.h>
#include "main.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////

MainFrm::MainFrm() {
    m_is_stop = false;
}

void MainFrm::run_forever() {
    // Url
    const std::string ws_url = "wss://api.upbit.com/websocket/v1";

    // Functions
    const auto func_open = [this](int ws_id) {
        printf("Websoket Opened\n");
        request_lob(ws_id);
    };

    const auto func_close = [this](int ws_id) {
        printf("Websoket Closed\n");
        m_is_stop = true;
    };

    const auto func_error = [this](int ws_id, std::string err_msg) {
        printf("Websoket Error, Msg: %s\n",  err_msg.c_str());
        m_is_stop = true;
    };

    const std::function<void(std::string)> func_rcv = std::bind(&MainFrm::on_ws_receive, this, std::placeholders::_1);
    //

    m_ws_mng.connect(ws_url, func_open, func_close, func_error, func_rcv);

    while (!m_is_stop) {
        sleep(10);
    }
}

long long MainFrm::get16dTs() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void MainFrm::request_lob(int ws_id) {
    std::string msg = R"([{"ticket":"tk"},{"type":"orderbook","codes":["KRW-BTC.1"]},{"format":"SIMPLE"}])";
    m_ws_mng.send(ws_id, msg);

    printf("Send Msg: %s\n", msg.c_str());
}

void MainFrm::on_ws_receive(std::string msg) {
    double bid_prc=0.0, bid_qty=0.0, ask_prc=0.0, ask_qty=0.0;
    long long st_ts=0;
    long long rj_ts=0, sj_ts=0;

    // RapidJson
    st_ts = get16dTs();

    rapidjson::Document doc_rj;
    doc_rj.Parse(msg.c_str());

    const rapidjson::Value& rv_obu_rj = doc_rj["obu"].GetArray();
    const rapidjson::Value& rv_data_rj = rv_obu_rj[0];

    bid_prc = rv_data_rj["bp"].GetDouble();
    bid_qty = rv_data_rj["bs"].GetDouble();
    ask_prc = rv_data_rj["ap"].GetDouble();
    ask_qty = rv_data_rj["as"].GetDouble();

    rj_ts = get16dTs() - st_ts;
    //

    // simdjson
    st_ts = get16dTs();

    simdjson::padded_string pad_str = simdjson::padded_string(msg);
    simdjson::ondemand::document doc_sj = m_parser_sj.iterate(pad_str);

    auto data_sj = doc_sj["obu"].get_array().at(0);

    bid_prc = data_sj["bp"];
    bid_qty = data_sj["bs"];
    ask_prc = data_sj["ap"];
    ask_qty = data_sj["as"];

    sj_ts = get16dTs() - st_ts;
    //

    // TODO
    
    printf("RapidJson: %lld, simdjson: %lld\n", rj_ts, sj_ts);
    //printf("%s\n", msg.c_str());
}

int main(int argc, char* argv[]) {
    std::setbuf(stdout, nullptr);

    MainFrm main_frm;
    main_frm.run_forever();

    return 0;
}
