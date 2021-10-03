#pragma once
#include <iostream>
#include <thread>
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include "../common/connection.h"

using namespace boost;

class Client {
public:
    ~Client();

    void connect(const std::string& host, std::string port);

    bool is_connected() const;

    void send_msg(const msg& m);

private:
    asio::io_context m_Context;
    std::unique_ptr<connection> m_Connection;

    std::thread m_Thread;

    ts_queue<msg_owner> m_QueueMsgIn;
};
