#include <chrono>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <set>
#include <boost/uuid/uuid_io.hpp>
#include "Server.h"

Server::Server(const property_tree::ptree& config) :
	//config cache
	m_Config(config),
	m_Timeout(m_Config.get<int>("timeout")),
	m_FileSize(m_Config.get<int>("file_size")),
	m_OutputDir(m_Config.get<std::string>("output_dir")),
	m_FilePrefix(m_Config.get<std::string>("file_prefix")),

	m_Acceptor(m_Context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), config.get<int>("port")))
{
	start();
}

Server::~Server()
{
	//joins both threads
	m_Context.stop();
	if (m_RunThread.joinable()) m_RunThread.join();

	m_StopConnectionThread = true;
	if (m_CheckConnectionsThread.joinable()) m_CheckConnectionsThread.join();

	std::cout << "[SERVER] Stopped\n";
}

void Server::start()
{
	try
	{
		//gives a task to the context before running so it doesn't die
		client_connection_task();
		//create the thread for the server context to run
		m_RunThread = std::thread([this]() { m_Context.run(); });
		//thread used to clean the closed connections
		m_CheckConnectionsThread = std::thread(
			[this]() 
			{ 
				//the atomic variable m_StopConnectionThread controls the loop life
				//during the server execution this thread should not die
				//it should only be joined at the destructor
				while (!m_StopConnectionThread)
				{
					//waits util m_Connections have at least one entry
					m_Connections.wait();
					//removes the closed connections
					m_Connections.remove_if(
						[](const std::shared_ptr<connection>& c) {
							return !c->is_connected();
						}
					);
					//sleep so the thread doesn't consume all the resources
					std::this_thread::sleep_for(std::chrono::seconds(10));
				}
			}
		);
	}
	catch (const std::exception& e)
	{
		std::cerr << "[SERVER] Exception at start: " << e.what() << "\n";
		return;
	}

	std::cout << "[SERVER] Started\n";
}

void Server::run()
{
	//waits util m_QueueMsgIn have at least one message
	m_QueueMsgIn.wait();

	//proccess all the messages
	while (!m_QueueMsgIn.empty())
	{
		//remove the front message and pass it to the handler function
		msg_owner msg = m_QueueMsgIn.pop_front();
		on_msg(msg);
	}
}

void Server::client_connection_task()
{
	m_Acceptor.async_accept(
		[this](std::error_code error, asio::ip::tcp::socket socket) 
		{
			if (!error) 
			{
				std::cout << "[SERVER] Connection: " << socket.remote_endpoint() << "\n";
				//adds the connection to the vector
				m_Connections.emplace_back(std::make_shared<connection>(connection::owner::server, m_Context, std::move(socket), m_QueueMsgIn, m_Timeout));
				//give it the task to wait the client's message
				m_Connections.back()->wait_to_client_msg_task();
			}
			else
			{
				std::cerr << "[SERVER] New Connection Error: " << error.message() << "\n";
			}

			//after we give the context a new task so it doesn't die
			client_connection_task();
		});
}

void Server::on_msg(const msg_owner& msgIn)
{
	//get the sent message
	std::string userMsg = msgIn.message.get();
	//get the client id
	std::string id = uuids::to_string(msgIn.owner->uuid());
	
	//current timestamp to string
	auto now = std::chrono::system_clock::now();
	auto tt = std::chrono::system_clock::to_time_t(now);
	auto localTime = std::localtime(&tt);

	std::stringstream ss;
	ss << localTime->tm_year + 1900 
	<< localTime->tm_mon + 1 
	<< localTime->tm_mday
	<< localTime->tm_hour
	<< localTime->tm_min
	<< localTime->tm_sec;
	std::string time = ss.str();

	//this set is used to order the files inside the directory so we get
	//the last one created
	std::set<std::filesystem::path> files;

	//base path
	std::filesystem::path path = m_OutputDir + "/" + id;
	//creates the base path for safety
	std::filesystem::create_directory(path);
	//iterator for the directory
	std::filesystem::directory_iterator directory(path);
	//append the file info to the path
	path += "/" + m_FilePrefix + "_" + time + ".txt";

	//goes through each entry in the directory
	for (const auto& entry : directory)
	{
		//checks if it's a file and 
		//if the current size + message size will not exceed the max size
		if (!entry.is_regular_file()) continue;
		if (entry.file_size() + userMsg.size() > m_FileSize) continue;
		//inserts in the set
		files.insert(entry);
	}
	
	//if anything was added to the set it means that they are
	//valid files, so we get the last one created
	//if none is added we are going to use the file name created earlier
	if (files.size() > 0)
		path = *(--files.end());

	//opens the file writes to it and close it
	std::ofstream file(path, std::ios::app);
	file << userMsg << "\n";
	file.close();

	//log the sent message to the console
	std::cout << "[" << id << "] New message: " << userMsg << "\n";
}
