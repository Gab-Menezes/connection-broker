#include "msg.h"

void msg::set(const char* data)
{
	//gets the string length + 1 (null terminator character)
	//set the header size
	//resize the vector
	//copy the data to to vector
	uint32_t s = strlen(data) + 1;
	header.size = s;
	body.resize(s);
	memcpy(body.data(), data, s);
}

void msg::set(const std::string& data)
{
	//gets the string length + 1 (null terminator character)
	//set the header size
	//resize the vector
	//copy the data to to vector
	uint32_t s = data.length() + 1;
	header.size = s;
	body.resize(s);
	memcpy(body.data(), data.c_str(), s);
}

std::string msg::get() const
{
	//reinterpret cast it to a char* that can be casted to a std::string
	return std::string(reinterpret_cast<const char*>(body.data()));
}
