// main.cpp
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <unistd.h>
#include <string>
#include <functional>
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

void request_lob(int ws_id) {
    // TODO

    std::string msg = "";

    m_ws_mng.send(ws_id, msg);
}

void MainFrm::on_ws_receive(std::string msg) {
    // TODO
    
    printf("%s\n", msg.c_str());
}

int main(int argc, char* argv[]) {
    std::setbuf(stdout, nullptr);

    MainFrm main_frm;
    main_frm.run_forever();

    return 0;
}
