#include "Client.h"

Client::~Client()
{
    //disconnect from the server
    //stop the context so we can join the run thread
    //release the connection pointer

    if (is_connected()) m_Connection->disconnect();

    m_Context.stop();

    if (m_Thread.joinable()) m_Thread.join();

    m_Connection.release();
}

void Client::connect(const std::string& host, std::string port)
{
    try
    {
        //adds a resolver to the context and resolve
        //the server host:port
        asio::ip::tcp::resolver resolver(m_Context);
        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, port);

        //create a connection pointer and give it the
        //task to connect to the server before running
        //so it doesn't die
        m_Connection = std::make_unique<connection>(connection::owner::client, m_Context, asio::ip::tcp::socket(m_Context), m_QueueMsgIn);
        m_Connection->connect_to_server_task(endpoints);

        //start the thread context
        m_Thread = std::thread([this]() { m_Context.run(); });
    }
    catch (const std::exception& e)
    {
        std::cerr << "[Client] Exception: " << e.what() << "\n";
        return;
    }

    std::cout << "[Client] Connected to server\n";
}

bool Client::is_connected() const
{
    if (m_Connection) return m_Connection->is_connected();
    else return false;
}

void Client::send_msg(const msg& m)
{
    if (is_connected()) m_Connection->send_msg(m);
}