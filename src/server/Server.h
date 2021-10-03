#pragma once
#include <iostream>
#include <memory>
#include <thread>
#include <atomic>
#include <deque>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include "../common/ts_vector.h"
#include "../common/connection.h"

using namespace boost;

class Server {
public:
    Server(const property_tree::ptree& config);
    ~Server();

    void run();
private:
    void start();
    
    //------------- TASKS ---------------
    //task to await a new client connection
    void client_connection_task();
    //------------- TASKS ---------------

    //messages handler function
    void on_msg(const msg_owner& msgIn);

    //asio
    asio::io_context m_Context;
    asio::ip::tcp::acceptor m_Acceptor;
    ts_vector<std::shared_ptr<connection>> m_Connections;
    ts_queue<msg_owner> m_QueueMsgIn;

    //threads
    std::thread m_RunThread;
    std::thread m_CheckConnectionsThread;
    std::atomic<bool> m_StopConnectionThread;

    //configuration
    const property_tree::ptree& m_Config;
    const int m_Timeout;
    const int m_FileSize;
    const std::string m_OutputDir;
    const std::string m_FilePrefix;
};
