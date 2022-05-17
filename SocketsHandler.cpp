#include "SocketsHandler.h"

void SocketsHandler::breakdownRequest(int idx)
{	
	SocketState& sock = sockets[idx];
	sock.Send = SEND;
	//fill request fields here:
	sock.Request.classifyReqMethod();//which method
	sock.Request.extractHeaders();//set the rest of the headers

}
void SocketsHandler::generateResponse(int idx)
{
	// response generated here valid/invalid
	sockets[idx].Response.reset();//<-- clean response fields, prior filling them
	if (isValidRequest(idx)) generateValidResponse(idx);
	else generateINValidResponse(idx);
	sockets[idx].Request.reset(); //<--- clean request fields, prepare for next request
}


bool SocketsHandler::canCreateFile(int idx)
{
	bool cancreate = false;
	std::ifstream checker(src_path + sockets[idx].Request.path);
	cancreate = !checker.is_open();
	if (!cancreate)   checker.close();
	return cancreate;
}


bool SocketsHandler::isValidRequest(int idx)
{
	SocketState& metaSocket = sockets[idx];
	request& cur_req = metaSocket.Request;
	response cur_res = metaSocket.Response;

	//are headers adequate for request method
	if (!cur_req.validateHeaders()) return false;
	
	switch (cur_req.reqMethod)
	{
	case UNSUPPORTED:
		return false;
	case GET:
	case HEAD:
		return cur_req.path != "";
	case TRACE:
		return cur_req.path == "echo";
	case POST:
		return canCreateFile(idx);
	case _DELETE:
		return !canCreateFile(idx);
	}
}
void SocketsHandler::generateValidResponse(int idx)
{
	bool success = false;
	SocketState& current_socket = sockets[idx];
	response& curr_res = current_socket.Response;
	request& curr_req = current_socket.Request;
	curr_res.host = curr_req.Host;
	switch (curr_req.reqMethod)
	{
	case GET:
		
		if (success = curr_res.extract_desired_file_content(src_path+curr_req.path))
		{
			curr_res.responseMSG.append(string("200") + CRLF);
		}
		
		else
			curr_res.responseMSG.append(string("500") + CRLF);
		curr_res.insertHeaders("GET",curr_req.path);
		if (success)
			curr_res.responseMSG.append(curr_res.bodyhandler);
		break;
	case POST:
		//create file (that doesn't exist enumarated) and  create resource (params?)
		//add status code based on previous action
		//add relevant headers : date,connection,cont-type,cont-length
		break;
	case PUT:
		//create file (or truncate a one that exists) and put body
		//add status code based on previous action
		//add relevant headers
		break;
	case _DELETE:
		// delete file 
		//add status code based on previous action
		//add relevant headers
		break;
	case HEAD:
		if (success = curr_res.extract_desired_file_content(src_path + curr_req.path))
		{
			curr_res.responseMSG.append(string("200") + CRLF);
		}

		else
			curr_res.responseMSG.append(string("500") + CRLF);
		curr_res.insertHeaders("GET", curr_req.path);
		break;
	case OPTIONS:
		curr_res.responseMSG.append(string("200") + CRLF);
		curr_res.insertHeaders("OPTIONS");
		break;
	case TRACE:
		curr_res.responseMSG.append(string("200") + CRLF);
		curr_res.insertHeaders("TRACE", {}, true,curr_req.raw_message.length());
		curr_res.responseMSG.append(curr_req.raw_message);
		break;
	}
}

void SocketsHandler::generateINValidResponse(int idx)
{
	SocketState& current_socket = sockets[idx];
	request& curr_req = current_socket.Request;
	response& curr_res = current_socket.Response;
	switch (curr_req.reqMethod)
	{
	case HEAD:
	case GET:
		if (curr_req.reqErr == NotFound || curr_req.reqErr == MissingData)
		curr_res.responseMSG.append(string("404 NOT FOUND") + CRLF);
		curr_res.insertHeaders("GET",curr_req.path,false);
		break;
	case POST:
		//create file (that doesn't exist) and  create resource (params?)
		//add status code based on previous action
		//add relevant headers : date,connection,cont-type,cont-length
		break;
	case PUT:
		//create file (or truncate a one that exists) and put body
		break;
	case _DELETE:

		break;
	case TRACE:
		curr_res.responseMSG.append(string("405") + CRLF);
		curr_res.insertHeaders("TRACE", {}, false);
		break;
	}
}

