// ws_mng.h
//////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//#include "zlib.h"  // 압축지원
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/common/memory.hpp>

//////////////////////////////////////////////////////////////////////////////////////////////////////

class ConnMetadata {
public:
    typedef websocketpp::lib::shared_ptr<ConnMetadata> ptr;

    ConnMetadata(int id, websocketpp::connection_hdl hdl, const std::function<void(int)> cbf_on_open, const std::function<void(int)> cbf_on_close, const std::function<void(int, std::string)> cbf_on_fail, const std::function<void(std::string)> cbf_on_msg);

    void on_open(websocketpp::client<websocketpp::config::asio_tls_client>* client, websocketpp::connection_hdl hdl);
    void on_close(websocketpp::client<websocketpp::config::asio_tls_client>* client, websocketpp::connection_hdl hdl);
    void on_fail(websocketpp::client<websocketpp::config::asio_tls_client>* client, websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, websocketpp::client<websocketpp::config::asio_tls_client>::message_ptr msg);

    bool on_ping(websocketpp::connection_hdl hdl, std::string msg);

    bool is_opened() { return m_is_opened; }
    websocketpp::connection_hdl get_hdl() { return m_hdl; }
    int get_id() { return m_id; }

protected:
    int m_id;
    bool m_is_opened;
    websocketpp::connection_hdl m_hdl;

    const std::function<void(int)> m_cbf_on_open;
    const std::function<void(int)> m_cbf_on_close;
    const std::function<void(int, std::string)> m_cbf_on_fail;
    const std::function<void(std::string)> m_cbf_on_msg;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////

class WsMng {
public:
    WsMng();
    virtual ~WsMng();

    std::shared_ptr<boost::asio::ssl::context> on_tls_init(websocketpp::connection_hdl);

    int connect(const std::string& uri, const std::function<void(int)> cbf_on_open, const std::function<void(int)> cbf_on_close, const std::function<void(int, std::string)> cbf_on_fail, const std::function<void(std::string)> cbf_on_msg);
    void send(int id, std::string message);

protected:
    websocketpp::client<websocketpp::config::asio_tls_client> m_endpoint;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

    std::map<int, ConnMetadata::ptr> m_connection_list;
    int m_next_id;
};
