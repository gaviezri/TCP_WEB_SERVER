#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <sstream>
#include <fstream>
#include <set>
class response
{
public:
	const char* CRLF = "\r\n";
	const std::string supported_methods = "GET, POST, PUT, DELETE, HEAD, OPTIONS, TRACE\r\n";
	std::string host;
	std::string responseMSG;
	std::string bodyhandler;
	void reset()
	{
		responseMSG = { "HTTP/1.1 " };
	}
	bool extract_desired_file_content(std::string);


	void insertHeaders(const char*, std::string = {}, bool valid = true,int=0);



};



