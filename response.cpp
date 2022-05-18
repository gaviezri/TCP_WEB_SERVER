#include "response.h"

bool response::extract_desired_file_content(std::string path)
{	
	
	std::ifstream requestedfile(path, std::ios_base::in);
	if (requestedfile.is_open())
	{
		std::stringstream rdbuf;
		rdbuf << requestedfile.rdbuf();
		bodyhandler = rdbuf.str();
		requestedfile.close();
		return true;
	}
	return false;
}


void response::insertHeaders(const char* method, std::string path,bool valid, int reqsize)
{
	std::set<std::string> CRUDpool = { "GET","HEAD","DELETE","PUT","POST", "TRACE"};
	time_t now = time(0);
	tm* gmtm = gmtime(&now);
	char* now_gmt = asctime(gmtm);
	now_gmt[strlen(now_gmt) - 1] = '\0';
	responseMSG.append(std::string("Date: ")+now_gmt+CRLF).append(std::string("Connection: keep-alive")+CRLF);
	responseMSG.append(std::string("Server: MyTCPServ") + CRLF);
	responseMSG.append(std::string("Host: ")+host+CRLF);

	if (std::string(method) == "OPTIONS")
	{
		responseMSG.append("Allow: ").append(supported_methods);
	}

	else if (CRUDpool.find(method)!=CRUDpool.end() && valid)
	{
		if (path.find(".txt") != std::string::npos)
			responseMSG.append(std::string("Content-Type: text/plain;charset=utf-8") + CRLF);
		else if (path.find(".html") != std::string::npos)
			responseMSG.append(std::string("Content-Type: text/html;charset=utf-8") + CRLF);
		else
			responseMSG.append(std::string("Content-Type: */*;charset=utf-8") + CRLF);
		if (std::string(method) != "HEAD" && valid)
		{
			responseMSG.append("Content-Length: ").append(std::to_string(method != "TRACE" ? 
				bodyhandler.length() : reqsize) + CRLF);
		}
		
	}
	responseMSG.append(CRLF);//extra empty line to indicate beggining of body
}
