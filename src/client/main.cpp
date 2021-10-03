#include <iostream>
#include <string>
#include "Client.h"

int main(int argc, char* argv[])
{
	std::string port = argc < 2 ? "8080" : argv[1];
	//creates the client and connect to the server
	Client client;
	client.connect("127.0.0.1", port);

	//create a message and string object so it can
	//be used for message input from the console
	msg m;
	std::string s;
	while (true)
	{
		//if connected we get the message from the
		//console and send it
		if (client.is_connected())
		{
			std::cout << "Message: ";
			std::cin >> s;
			m.set(s);
			client.send_msg(m);
		}
		else
			break;
	}

	return 0;
}
