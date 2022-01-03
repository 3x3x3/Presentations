// ws_mng.cpp
//////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ws_mng.h"
#include <string>
#include <websocketpp/common/thread.hpp>

//////////////////////////////////////////////////////////////////////////////////////////////////////

ConnMetadata::ConnMetadata(int id, websocketpp::connection_hdl hdl, const std::function<void(int)> cbf_on_open, const std::function<void(int)> cbf_on_close, const std::function<void(int, std::string)> cbf_on_fail, const std::function<void(std::string)> cbf_on_msg)
    : m_id(id), m_hdl(hdl), m_is_opened(false), m_cbf_on_open(cbf_on_open), m_cbf_on_close(cbf_on_close), m_cbf_on_fail(cbf_on_fail), m_cbf_on_msg(cbf_on_msg) {
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void ConnMetadata::on_open(websocketpp::client<websocketpp::config::asio_tls_client>* client, websocketpp::connection_hdl hdl) {
    m_is_opened = true;
    m_cbf_on_open(m_id);
}

void ConnMetadata::on_close(websocketpp::client<websocketpp::config::asio_tls_client>* client, websocketpp::connection_hdl hdl) {
    m_is_opened = false;
    m_cbf_on_close(m_id);
}

void ConnMetadata::on_fail(websocketpp::client<websocketpp::config::asio_tls_client>* client, websocketpp::connection_hdl hdl) {
    m_is_opened = false;

    websocketpp::client<websocketpp::config::asio_tls_client>::connection_ptr con = client->get_con_from_hdl(hdl);
    const std::string buffer = con->get_ec().message();
    m_cbf_on_fail(m_id, buffer);
}

void ConnMetadata::on_message(websocketpp::connection_hdl hdl, websocketpp::client<websocketpp::config::asio_tls_client>::message_ptr msg) {
    const websocketpp::frame::opcode::value opcode = msg->get_opcode();

    if ( websocketpp::frame::opcode::binary == opcode || websocketpp::frame::opcode::text == opcode ) {
        const std::string buffer = msg->get_payload();
        m_cbf_on_msg(buffer);
    }
}

bool ConnMetadata::on_ping(websocketpp::connection_hdl hdl, std::string msg) {
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

WsMng::WsMng() : m_next_id(0) {
    m_endpoint.set_access_channels(websocketpp::log::alevel::all);
    m_endpoint.set_error_channels(websocketpp::log::elevel::all);

    m_endpoint.clear_access_channels(websocketpp::log::alevel::control);
    m_endpoint.clear_access_channels(websocketpp::log::alevel::frame_header);
    m_endpoint.clear_access_channels(websocketpp::log::alevel::frame_payload);
    
    m_endpoint.init_asio();
    m_endpoint.set_tls_init_handler(websocketpp::lib::bind(&WsMng::on_tls_init, this, websocketpp::lib::placeholders::_1));
    m_endpoint.start_perpetual();
    
    m_thread.reset(new websocketpp::lib::thread(&websocketpp::client<websocketpp::config::asio_tls_client>::run, &m_endpoint));
}

WsMng::~WsMng() {
    m_endpoint.stop_perpetual();

    for (std::map<int,ConnMetadata::ptr>::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
        if ( !it->second->is_opened() ) {
            // Only close open connections
            continue;
        }
        
        std::cout << "> Closing connection " << it->second->get_id() << std::endl;
        
        websocketpp::lib::error_code ec;
        m_endpoint.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
        if (ec) {
            std::cout << "> Error closing connection " << it->second->get_id() << ": " << ec.message() << std::endl;
        }
    }
    
    m_thread->join();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<boost::asio::ssl::context> WsMng::on_tls_init(websocketpp::connection_hdl) {
    std::shared_ptr<boost::asio::ssl::context> ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tls);

    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds | 
                        boost::asio::ssl::context::no_sslv2 |
                        boost::asio::ssl::context::no_sslv3 | 
                        boost::asio::ssl::context::single_dh_use
        );

        ctx->set_verify_mode(boost::asio::ssl::verify_none);
    } catch (std::exception &e) {
        std::cout << "Error in context pointer: " << e.what() << std::endl;;
    }
    return ctx;
}

int WsMng::connect(const std::string& uri, const std::function<void(int)> cbf_on_open, const std::function<void(int)> cbf_on_close, const std::function<void(int, std::string)> cbf_on_fail, const std::function<void(std::string)> cbf_on_msg) {
    websocketpp::lib::error_code ec;
    websocketpp::client<websocketpp::config::asio_tls_client>::connection_ptr con = m_endpoint.get_connection(uri, ec);

    if (ec) {
        std::cout << "> Connect initialization error: " << ec.message() << std::endl;
        return -1;
    }

    int new_id = m_next_id++;
    ConnMetadata::ptr metadata_ptr(new ConnMetadata(new_id, con->get_handle(), cbf_on_open, cbf_on_close, cbf_on_fail, cbf_on_msg));
    m_connection_list[new_id] = metadata_ptr;

    con->set_open_handler(websocketpp::lib::bind(
        &ConnMetadata::on_open,
        metadata_ptr,
        &m_endpoint,
        websocketpp::lib::placeholders::_1
    ));
    
    con->set_fail_handler(websocketpp::lib::bind(
        &ConnMetadata::on_fail,
        metadata_ptr,
        &m_endpoint,
        websocketpp::lib::placeholders::_1
    ));

    con->set_close_handler(websocketpp::lib::bind(
        &ConnMetadata::on_close,
        metadata_ptr,
        &m_endpoint,
        websocketpp::lib::placeholders::_1
    ));

    con->set_message_handler(websocketpp::lib::bind(
        &ConnMetadata::on_message,
        metadata_ptr,
        websocketpp::lib::placeholders::_1,
        websocketpp::lib::placeholders::_2
    ));

    con->set_ping_handler(websocketpp::lib::bind(
        &ConnMetadata::on_ping,
        metadata_ptr,
        websocketpp::lib::placeholders::_1,
        websocketpp::lib::placeholders::_2
    ));

    m_endpoint.connect(con);

    return new_id;
}

void WsMng::send(int id, std::string message) {
    websocketpp::lib::error_code ec;
    
    std::map<int,ConnMetadata::ptr>::const_iterator metadata_it = m_connection_list.find(id);
    if (metadata_it == m_connection_list.end()) {
        std::cout << "> No connection found with id " << id << std::endl;
        return;
    }
    
    m_endpoint.send(metadata_it->second->get_hdl(), message, websocketpp::frame::opcode::text, ec);
    if (ec) {
        std::cout << "> Error sending message: " << ec.message() << std::endl;
        return;
    }
}