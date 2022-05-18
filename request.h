#pragma once
#include <string>
#include <fstream>
#include <set>
using std::string;


enum eReqMethod { GET = 0, HEAD, OPTIONS, POST, PUT, _DELETE, TRACE, UNSUPPORTED, NONE };
enum eReqError { UnsupportedHeader, UnsupportedMethod,UnsupportedFormat ,NotFound, MissingData ,NOERR };
class request
{
public:
	const char* CRLF = "\r\n";
	eReqError reqErr;
	eReqMethod reqMethod;
	//----------
	string raw_message;
	//----------
	const char* src_path = "c:/tmp/";
	string path;
	string Query;
	//-----------
	string Host; 
	string Connection;
	//-----------
	string Date;
	string Accept;
	string Accept_Lang;
	string Accept_charset;
	string Content_Type;
	size_t Content_Length;
	//---------------
	string Body;
	int read_cursor;


	string getFieldbyIdx(size_t,size_t);
	void classifyReqMethod()
	{

		if (!raw_message.compare(0, 3, "GET"))
		{
			reqMethod = GET;
			read_cursor= 5;
		}
		else if (!raw_message.compare(0, 4, "POST"))
		{
			reqMethod = POST;
			read_cursor= 6;
		}
		else if (!raw_message.compare(0, 3, "PUT"))
		{
			reqMethod = PUT;
			read_cursor= 5;
		}
		else if (!raw_message.compare(0, 4, "HEAD"))
		{
			reqMethod = HEAD;
			read_cursor= 6;
		}
		else if (!raw_message.compare(0, 6, "DELETE"))
		{
			reqMethod = _DELETE;
			read_cursor= 8;
		}
		else if (!raw_message.compare(0, 7, "OPTIONS"))
		{
			reqMethod = OPTIONS;
			read_cursor= 9;
		}
		else if (!raw_message.compare(0, 5, "TRACE"))
		{
			reqMethod = TRACE;
			read_cursor= 7;
		}
		else
		{
			reqMethod = UNSUPPORTED;
			read_cursor= 0;
		}
	}
	void extractHeaders();
	void assignReqHeaders();
	void appendQuerytoPath(string);
	void reset()
	{
		reqMethod = NONE;
		reqErr = NOERR;
		raw_message.clear();
		path.clear();
		Query.clear();
		Connection.clear();
		Host.clear();
		Accept.clear();
		Accept_Lang.clear();
		Content_Type.clear();
		Content_Length = 0;
		read_cursor = 0;
	}
	bool validateHeaders();
	bool createResource();
	bool deleteResource();
};


