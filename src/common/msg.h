#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <cstring>

//message represantation
//we use a header because we know it has a fixed number of bytes
//in it we add the body size so we can resize the body vector to the
//correct size

struct msg_header
{
	uint32_t size = 0;
};

struct msg
{
	msg_header header{};
	//uint8_t (byte) is used to represent any arbitrary data
	std::vector<uint8_t> body;

	//set the body with const char*
	void set(const char* data);

	//set the body with std::string
	void set(const std::string& data);

	//get the message out of the vector
	std::string get() const;
};

//ahead declaration of the connection class
class connection;

//message + owner
struct msg_owner
{
	std::shared_ptr<connection> owner;
	msg message;
};
