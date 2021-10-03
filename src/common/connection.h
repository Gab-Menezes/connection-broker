#pragma once
#include <iostream>
#include <memory>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "msg.h"
#include "ts_queue.h"

using namespace boost;

class connection : public std::enable_shared_from_this<connection>
{
public:
    enum class owner
    {
        server, client
    };

    connection(owner o, asio::io_context& context, asio::ip::tcp::socket&& socket, ts_queue<msg_owner>& msgIn, int timeout = 1);

    //connection uuid
    const boost::uuids::uuid& uuid() const;

    //is the connection alive
    bool is_connected() const;

    //closes the connection if open
    void disconnect();

    //function used as the callback for time out
    void disconnect_timer(const boost::system::error_code& e);

    //------------- TASKS ---------------
    //after the client connection we wait for a message
    void wait_to_client_msg_task();

    //connects the client to the server
    void connect_to_server_task(const asio::ip::tcp::resolver::results_type& endpoints);
    //------------- TASKS ---------------

    //adds a new message to the out message queue
    //and dispatch a task to write it
    void send_msg(const msg& m);

private:
    //------------- TASKS ---------------
    //task responsible to await for a new message and read it's header
    void read_header_task();

    //task responsible to await for the body and read it after the header has been read
    void read_body_task();

    //task responsible for writing the header of the sent message
    void write_header_task();

    //task responsible for writing the body of the sent message after the header has been written
    void write_body_task();

    //task responsible for pushing the incoming message to the queue
    void push_to_msg_queue_task();
    //------------- TASKS ---------------

    //asio
    asio::io_context& m_Context;
    asio::ip::tcp::socket m_Socket;
    boost::asio::steady_timer m_Timer;
    int m_Timeout;

    //infomation
    owner m_Owner;
    boost::uuids::uuid m_Uuid;

    //messages
    msg m_TempMsg;
    ts_queue<msg_owner>& m_QueueMsgIn;
    ts_queue<msg> m_QueueMsgOut;

};
