// main.cpp
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <unistd.h>
#include <string>
#include <functional>
#include <chrono>
#include "main.h"
#include "custom_parser.h"

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

long long MainFrm::get_16d_ts() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void MainFrm::request_lob(int ws_id) {
    std::string msg = R"([{"ticket":"tk"},{"type":"orderbook","codes":["KRW-BTC.1"]},{"format":"SIMPLE"}])";
    m_ws_mng.send(ws_id, msg);

    printf("Send Msg: %s\n", msg.c_str());
}

void MainFrm::on_ws_receive(std::string msg) {
    printf("%s\n", msg.c_str());

    std::string_view asset_code[3];
    double bid_prc[3]={0.0,}, bid_qty[3]={0.0,}, ask_prc[3]={0.0,}, ask_qty[3]={0.0,};
    long long st_ts=0;
    long long calc_ts[3]={0,};

    // RapidJson
    st_ts = get_16d_ts();

    m_doc_rj.Parse(msg.c_str());

    const rapidjson::Value& rv_obu_rj = m_doc_rj["obu"].GetArray();
    const rapidjson::Value& rv_data_rj = rv_obu_rj[0];

    asset_code[0] = m_doc_rj["cd"].GetString();
    ask_prc[0] = rv_data_rj["ap"].GetDouble();
    bid_prc[0] = rv_data_rj["bp"].GetDouble();
    ask_qty[0] = rv_data_rj["as"].GetDouble();
    bid_qty[0] = rv_data_rj["bs"].GetDouble();

    calc_ts[0] = get_16d_ts() - st_ts;
    //

    // simdjson
    st_ts = get_16d_ts();

    std::string_view tmp = msg;
    simdjson::ondemand::document doc_sj = m_parser_sj.iterate(simdjson::padded_string_view(tmp, 256));

    asset_code[1] = std::string_view(doc_sj["cd"]);
    auto data_sj = doc_sj["obu"].get_array().at(0);
    ask_prc[1] = data_sj["ap"].get_double();
    bid_prc[1] = data_sj["bp"].get_double();
    ask_qty[1] = data_sj["as"].get_double();
    bid_qty[1] = data_sj["bs"].get_double();

    calc_ts[1] = get_16d_ts() - st_ts;
    //

    // JSONTokenizer
    st_ts = get_16d_ts();
    int parsing_cnt = 0;

    JSONTokenizer tokenizer(msg);
    while(tokenizer) {
        const char* token = tokenizer.front().data();
 
        if ( 'c' == token[0] && 'd' == token[1] ) {
            tokenizer.pop();
            asset_code[2] = tokenizer.front();
            parsing_cnt++;
        }
        else if ( 'b' == token[0] && 'p' == token[1] ) {
            tokenizer.pop();
            bid_prc[2] = std::stod(tokenizer.front().data());
            parsing_cnt++;
        }
        else if ( 'b' == token[0] && 's' == token[1] ) {
            tokenizer.pop();
            bid_qty[2] = std::stod(tokenizer.front().data());
            parsing_cnt++;
        }
        else if ( 'a' == token[0] && 'p' == token[1] ) {
            tokenizer.pop();
            ask_prc[2] = std::stod(tokenizer.front().data());
            parsing_cnt++;
        }
        else if ( 'a' == token[0] && 's' == token[1] ) { 
            tokenizer.pop();
            ask_qty[2] = std::stod(tokenizer.front().data());
            parsing_cnt++;
        }
        
        if ( 5 == parsing_cnt ) {
            break;
        }

        tokenizer.pop();
    }

    calc_ts[2] = get_16d_ts() - st_ts;
    //

    /*
    // JSONParser
    st_ts = get_16d_ts();

    JSONParser parser(msg);
    int ix=0;
    parser.consume(ix);

    int cur = 1;
    StrPos cd_pos = parser.find_value_pos(cur, "cd");
    asset_code[2] = std::string_view(&parser.buf[cd_pos.begin+1], cd_pos.end-cd_pos.begin-1);
    StrPos obu_pos = parser.find_value_pos(cur, "ob");
    StrPos lob2 = parser.find_value_pos(obu_pos.begin+1, 2);

    ask_prc[2] = parser.find_value(lob2.begin+1, "ap");
    bid_prc[2] = parser.find_value(lob2.begin+1, "bp");
    ask_qty[2] = parser.find_value(lob2.begin+1, "as");
    bid_qty[2] = parser.find_value(lob2.begin+1, "bs");

    calc_ts[2] = get_16d_ts() - st_ts;
    //
    */
    
    printf("RapidJson: %lld, simdjson: %lld, JSONtokenizer: %lld\n", calc_ts[0], calc_ts[1], calc_ts[2]);

    for ( int i=0 ; i<3 ; i++ ) {
        std::string cd = std::string(asset_code[i].data(), asset_code[i].size());
        printf("idx: %d, cd: %s, ap: %lf, bp: %lf, aq: %lf, bq: %lf\n", i, cd.c_str(), ask_prc[i], bid_prc[i], ask_qty[i], bid_qty[i]);
    }
}

int main(int argc, char* argv[]) {
    std::setbuf(stdout, nullptr);

    MainFrm main_frm;
    main_frm.run_forever();

    return 0;
}
