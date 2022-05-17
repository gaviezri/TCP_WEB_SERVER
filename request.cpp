#include "request.h"

bool request::fileExist(std::string path)
{
	bool exist = false;
	std::ifstream checker(src_path + path,std::ios_base::in);
	exist = checker.is_open();
	if (exist)   checker.close();
	return exist;
}
string request::getFieldbyIdx(size_t idx,size_t fieldbias)
{
	if(idx-fieldbias!=string::npos)
		return raw_message.substr(idx, raw_message.substr(idx).find(CRLF));
	return {};
}
void request::assignReqHeaders()
{
	//make all content lower-case
	for (auto& c : raw_message) c = tolower(c);

	size_t host_idx = raw_message.find("host:") + 5;
	size_t connection_idx = raw_message.find("connection:") + 11;
	size_t accept_idx = raw_message.find("accept:") + 7;
	size_t accept_lang_idx = raw_message.find("accept-language:") + 16;
	size_t date_idx = raw_message.find("date:") + 5;
	size_t cont_type_idx = raw_message.find("content-type:") + 13;
	size_t content_len_idx = raw_message.find("content-length:") + 15;
	size_t charset_idx = raw_message.find("accept-charset:") + 15;
	size_t body_idx = raw_message.find(string(CRLF) + CRLF)+4;

	Host = getFieldbyIdx(host_idx,5);
	Connection = getFieldbyIdx(connection_idx,11);
	Accept = getFieldbyIdx(accept_idx,7);
	Accept_Lang = getFieldbyIdx(accept_lang_idx,16);
	Content_Type = getFieldbyIdx(cont_type_idx,13);
	Content_Length = atol(getFieldbyIdx(content_len_idx,15).c_str());
	Date = getFieldbyIdx(date_idx,5);
	Accept_charset = getFieldbyIdx(charset_idx,15);
	Body = getFieldbyIdx(body_idx,4);
}

void request::appendQuerytoPath(string lang)
{
	std::string extension;
	if (path.find(".txt") != std::string::npos) extension = ".txt";
	else if (path.find(".html") != std::string::npos) extension = ".html";
	size_t extension_idx = path.find_last_of(".");
	path = path.substr(0, extension_idx);
	path.append("-"+lang).append(extension);
}
void request::extractHeaders()
{
	if (reqMethod == UNSUPPORTED) return;
	assignReqHeaders();
	std::set<eReqMethod> method_pool = { GET,PUT,POST,_DELETE,HEAD,TRACE};
	if (method_pool.find(reqMethod)!=method_pool.end())
	{
	     int querystart = raw_message.find('?');
	     int endofpath = raw_message.substr(read_cursor).find(' ');
	     int path_len = querystart == string::npos ?
			endofpath : abs(read_cursor - querystart);
		path = raw_message.substr(read_cursor, path_len);
		read_cursor += path_len;
		if (querystart != string::npos)
			Query = raw_message.substr(querystart+1, raw_message.substr(querystart).find(' ')-1);
		else if (path == "")	reqErr = MissingData;
	}
	if (Query == "lang=en") appendQuerytoPath("en");
	else if (Query == "lang=he") appendQuerytoPath("he");
	else if (Query == "lang=fr") appendQuerytoPath("fr");
}

bool request::validateHeaders()
{
	std::set<eReqMethod> method_pool = { GET,PUT,POST,_DELETE,HEAD };
	if (reqMethod == UNSUPPORTED) return false;

	if (Host == "") return false;
	//check for required information provided for method
	if (reqMethod == PUT || reqMethod == POST)
	{
		if ((path == "" && Query == "") || Content_Length == 0)
		{
			reqErr = eReqError::MissingData;
			return false;
		}
	}
	else
	// check for redundancy provided 
	{
		if (Body != "" || Content_Length > 0)
		{
			reqErr = eReqError::UnsupportedHeader;
			return false;
		}
	}
	if (method_pool.find(reqMethod)!=method_pool.end())
	{
		if (path == "")
		{
			reqErr = MissingData;
			return false;
		}
		if(Query != "" && Query.find("lang") == string::npos)
		{
			reqErr = UnsupportedHeader;
			return false;
		}
	}
	if (reqMethod == TRACE)
		return path == "echo";
	//server supports only this type of content
	if (Content_Type != "text/html" && Content_Type != "text/plain" && Content_Type != "")
	{
		reqErr = eReqError::UnsupportedFormat;
		return false;
	}
	//server supports 3 specific languages
	if (Accept_Lang != "" &&
		Accept_Lang.find("en") == string::npos &&
		Accept_Lang.find("fr") == string::npos &&
		Accept_Lang.find("he") == string::npos)
	{
		reqErr = eReqError::NotFound;
		return false;
	}//server supports only following formats
	if (Accept != "" &&
		Accept.find("*/*") == string::npos &&
		Accept.find("text/html") == string::npos &&
		Accept.find("text/*") == string::npos &&
		Accept.find("text/plain") == string::npos)
	{
		reqErr = UnsupportedFormat;
		return false;
	}
	return true;
}