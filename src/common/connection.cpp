#include "connection.h"

connection::connection(owner o, asio::io_context& context, asio::ip::tcp::socket&& socket, ts_queue<msg_owner>& msgIn, int timeout) :
    m_Owner(o),
    m_Context(context),
    m_Socket(std::move(socket)),
    m_QueueMsgIn(msgIn),
    m_Timeout(timeout),
    m_Timer(context, std::chrono::minutes(timeout)),
    m_Uuid(boost::uuids::random_generator()())
{
}

const boost::uuids::uuid& connection::uuid() const
{
    return m_Uuid;
}

bool connection::is_connected() const
{
    return m_Socket.is_open();
}

void connection::disconnect()
{
    if (m_Owner == owner::server)
        std::cout << "[" << m_Uuid << "] disconnected\n";
    else
        std::cout << "Disconnected\n";

    if (is_connected())
        asio::post(m_Context, [this]() { m_Socket.close(); });
}

void connection::disconnect_timer(const boost::system::error_code& error)
{
    //if the proccess was not aborted (the timer expired) we disconnect
    //if not (the expiration time changed) we reset the timer
    if (!error) {
        disconnect();
    }
    else
        m_Timer.async_wait(boost::bind(&connection::disconnect_timer, this, asio::placeholders::error));
}

void connection::wait_to_client_msg_task()
{
    if (is_connected())
    {
        //starts the timer for client timeout
        m_Timer.async_wait(boost::bind(&connection::disconnect_timer, this, asio::placeholders::error));
        //give it task to wait for a new header
        read_header_task();
    }
    else
        std::cerr << "Failed connecting to the client: socket is disconnected\n";
}

void connection::connect_to_server_task(const asio::ip::tcp::resolver::results_type& endpoints)
{
    asio::async_connect(m_Socket, endpoints,
        [this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
        {
            //if the connection was successful we wait for the server to
            //send a new header, but in this case only the client
            //sends information, so this is used "fake" a real scenario
            if (!ec)
                read_header_task();
            else
                std::cerr << "Failed connecting to the server: " << ec.message() << "\n";
        });
}

void connection::send_msg(const msg& m)
{
    asio::post(m_Context, [this, &m]()
        {
            //we check if it's empty because if it's not
            //another message is already being processed
            //and we don't want to dispatch a new task

            //the new one will be automaticaly processed
            //after the oldest ones
            bool isEmpty = m_QueueMsgOut.empty();
            m_QueueMsgOut.push_back(m);
            if (isEmpty)
                write_header_task();
        });
}

void connection::read_header_task()
{
    asio::async_read(m_Socket, asio::buffer(&m_TempMsg.header, sizeof(msg_header)),
        [this](std::error_code error, std::size_t size)
        {
            //extends the timer expiration
            m_Timer.expires_at(std::chrono::steady_clock::now() + std::chrono::minutes(m_Timeout));

            if (!error)
            {
                //if the message has any content we need to resize the body vector
                //and dispatch the read body task
                //if we ignore and await for a new message
                if (m_TempMsg.header.size > 0)
                {
                    m_TempMsg.body.resize(m_TempMsg.header.size);
                    read_body_task();
                }
                else
                    read_header_task();
            }
            else
                std::cerr << "Failed to read the header: " << error.message() << "\n";
        }
    );
}

void connection::read_body_task()
{
    asio::async_read(m_Socket, asio::buffer(m_TempMsg.body.data(), m_TempMsg.body.size()), 
        [this](std::error_code error, std::size_t size)
        {
            //if everything is ok the read message is added to the queue
            if (!error)
                push_to_msg_queue_task();
            else
                std::cerr << "Failed to read the body: " << error.message() << "\n";
        }
    );
}

void connection::write_header_task()
{
    asio::async_write(m_Socket, asio::buffer(&m_QueueMsgOut.front().header, sizeof(msg_header)),
        [this](std::error_code error, size_t size)
        {
            if (!error)
            {
                //if the message first message body isn't empty we write it's body
                //if not we remove it and check there is any other message in
                //the queue, if yes we dispatch the write header task again for this message
                if (!m_QueueMsgOut.front().body.empty())
                    write_body_task();
                else
                {
                    m_QueueMsgOut.pop_front();
                    if (!m_QueueMsgOut.empty())
                        write_header_task();
                }
            }
            else
                std::cerr << "Failed to write the header: " << error.message() << "\n";
        }
    );
}

void connection::write_body_task()
{
    asio::async_write(m_Socket, asio::buffer(m_QueueMsgOut.front().body.data(), m_QueueMsgOut.front().body.size()),
        [this](std::error_code error, size_t size)
        {
            if (!error)
            {
                //since the full message (header + body) has been written
                //we can pop it and check if there is any other message
                //and if yes dispatch the write header task again
                m_QueueMsgOut.pop_front();

                if (!m_QueueMsgOut.empty())
                    write_header_task();
            }
            else
                std::cerr << "Failed to write the body: " << error.message() << "\n";
        }
    );
}

void connection::push_to_msg_queue_task()
{
    //if the message owner (who recived it) is the server
    //we pass a shared pointer of this object so we can have access to the
    //connection info when reading the message
    //-------
    //in the client case the pointer isn't passed because we alredy know about
    //the connection, sice it can only be the server
    if (m_Owner == owner::server)
        m_QueueMsgIn.push_back({ this->shared_from_this(), m_TempMsg });
    else
        m_QueueMsgIn.push_back({ nullptr, m_TempMsg });

    //dispatch a read header task for await new messages
    read_header_task();
}
