//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                  //
//         Triton Progressive Web-Server                                                            //
//                                                                                                  //
//      [100]   04/01/2020  Cold Re-Write Began                                                     //
//                                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
 
#include <winsock2.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "STRING.h"

#pragma comment(lib, "Ws2_32.lib")
 
#define SERVICE_PORT        80
#define NET_BUFFER_SIZE     4096

bool		Running;

const char *	Protocol = "HTTP/1.1";

bool ProcessRequest(SOCKET _Client);

int GetReadyForOffice()
{
WSADATA wsaData;
int     iResult;
 
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(iResult) 
	{
		printf("WSAStartup failed: %d\n", iResult);
		return(1);
	}
	return(0);
}

void CleanUpTheMess()
{
	WSACleanup();
}

const char *	CT_TEXT = "text/html";
const char *	CT_CSS = "css";

int LoadResourceFile(STRING & _URI,char * * page,const char * * _content)
{
HANDLE  hFile;
DWORD   readed,size = 0;

	*_content = (STRING(&_URI.String[_URI.Length() - 4]) == ".css") ? CT_CSS : CT_TEXT;
/*
	if(_URI == "LOCK")
	{
		if(Progressive->LockBets(TablePtr))
		{
			SendGoodXML();	// Respond that table lock allowed
		}
		else
		{
			SendFailXML();	// Respond that table lock NOT allowed
		}
	}
	else if(_URI == "UNLOCK")
	{
		Progressive->UnlockBets(TablePtr);
	}
	else
*/	{
		hFile = CreateFileA(&_URI.String[(*_URI.String == '/') ? 1 : 0],GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			size = SetFilePointer(hFile,0,NULL,SEEK_END);
			*page = new char[size];
			SetFilePointer(hFile,0,NULL,SEEK_SET);
			if(ReadFile(hFile,*page,size,&readed,NULL) == false)
			{
				printf("Failed to load resource file: %s\n",_URI.String);
				delete *page;
				*page = nullptr;
				size = 0;
			}

			CloseHandle(hFile);
		}
		else printf("Failed to locate resource file: %s\n",_URI.String);
	}

	return(size);
}

typedef enum
{
	RT_UNKNOWN = 0,
	RT_OPTIONS,
	RT_GET,
	RT_HEAD,
	RT_POST,
	RT_PUT,
	RT_DELETE,
	RT_TRACE,
	RT_CONNECT,

	REQ_TYPES

} REQ_TYPE;

struct RequestNames
{
	REQ_TYPE		Req;
	const char *	Name;

} rname[REQ_TYPES] =					{
											{	RT_UNKNOWN,		"UNKNOWN"	},
											{	RT_OPTIONS,		"OPTIONS"	},
											{	RT_GET,			"GET"		},
											{	RT_HEAD,		"HEAD"		},
											{	RT_POST,		"POST"		},
											{	RT_PUT,			"PUT"		},
											{	RT_DELETE,		"DELETE"	},
											{	RT_TRACE,		"TRACE"		},
											{	RT_CONNECT,		"CONNECT"	},
										};

#define	CHAR_LF	0x0A
#define	CHAR_CR	0x0D

bool EOL(char _ch)
{
	switch(_ch)
	{
	case 0x0A :		// LF
	case 0x0D :		// CR
		return(true);
	}
	return(false);
}

struct HTTP_Message
{
	STRING	Request;
	STRING	StatusLine;
	vSTRING	HeaderFields;
	vSTRING	MessageBody;

	STRING	Host;

	void	Parse(const char * _buffer);
};

typedef enum class HTTP_StatusCode
{
// Informational
	HTTPSC_Continue = 100,
	HTTPSC_SwitchingProtocols = 101,
// Successful
	HTTPSC_OK = 200,
	HTTPSC_Created = 201,
	HTTPSC_Acceted = 202,
	HTTPSC_NonAuthoritativeInformation = 203,
	HTTPSC_NoContent = 204,
	HTTPSC_ResetContent = 205,
	HTTPSC_ParialContent = 206,
// Redirection 3xx
	HTTPSC_MultipleChoices = 300,
	HTTPSC_MovedPermanently = 301,
	HTTPSC_Found = 302,
	HTTPSC_SeeOther = 303,
	HTTPSC_NotModified = 304,
	HTTPSC_UseProxy = 305,
	HTTPSC_Unused = 306,
	HTTPSC_TemporaryRedirect = 307,
// Client Error 4xx
	HTTPSC_BadRequest = 400,
	HTTPSC_Unauthorized = 401,
	HTTPSC_PaymentRequired = 402,
	HTTPSC_Forbidden = 403,
	HTTPSC_NotFound = 404,
	HTTPSC_MethodNotAllowed = 405,
	HTTPSC_NotAcceptable = 406,
	HTTPSC_ProxyAuthenticationRequired = 407,
	HTTPSC_RequestTimeout = 408,
	HTTPSC_Conflict = 409,
	HTTPSC_Gone = 410,
	HTTPSC_LengthRequired = 411,
	HTTPSC_PreconditionFailed = 412,
	HTTPSC_RequestEntityTooLarge = 413,
	HTTPSC_RequestURITooLong = 414,
	HTTPSC_UnsupportedMediaType = 415,
	HTTPSC_RequestRangeNotSatisfiable = 416,
	HTTPSC_ExpectationFailed = 417,
// Server Error 5xx
	HTTPSC_InternalServerError = 500,
	HTTPSC_NotImplemented = 501,
	HTTPSC_BadGateway = 502,
	HTTPSC_ServiceUnavailable = 503,
	HTTPSC_GatewayTimeout = 504,
	HTTPSC_HTTPVersionNotSupported = 505,

} HTTPSC;

struct HTTP_Status
{
	HTTPSC			Code;
	const char *	Text;
};

const HTTP_Status	STS[] = {
								{	HTTP_StatusCode::HTTPSC_Continue					,	"Continue"						},
								{	HTTP_StatusCode::HTTPSC_SwitchingProtocols			,	"Switching Protocols"			},
								{	HTTP_StatusCode::HTTPSC_OK							,	"OK"							},
								{	HTTP_StatusCode::HTTPSC_Created						,	"Created"						},
								{	HTTP_StatusCode::HTTPSC_Acceted						,	"Acceted"						},
								{	HTTP_StatusCode::HTTPSC_NonAuthoritativeInformation	,	"Non-Authoritative Information"	},
								{	HTTP_StatusCode::HTTPSC_NoContent					,	"No Content"					},
								{	HTTP_StatusCode::HTTPSC_ResetContent				,	"Reset Content"					},
								{	HTTP_StatusCode::HTTPSC_ParialContent				,	"Parial Content"				},
								{	HTTP_StatusCode::HTTPSC_MultipleChoices				,	"Multiple Choices"				},
								{	HTTP_StatusCode::HTTPSC_MovedPermanently			,	"Moved Permanently"				},
								{	HTTP_StatusCode::HTTPSC_Found						,	"Found"							},
								{	HTTP_StatusCode::HTTPSC_SeeOther					,	"See Other"						},
								{	HTTP_StatusCode::HTTPSC_NotModified					,	"Not Modified"					},
								{	HTTP_StatusCode::HTTPSC_UseProxy					,	"Use Proxy"						},
								{	HTTP_StatusCode::HTTPSC_Unused						,	"Unused"						},
								{	HTTP_StatusCode::HTTPSC_TemporaryRedirect			,	"Temporary Redirect"			},
								{	HTTP_StatusCode::HTTPSC_BadRequest					,	"Bad Request"					},
								{	HTTP_StatusCode::HTTPSC_Unauthorized				,	"Unauthorized"					},
								{	HTTP_StatusCode::HTTPSC_PaymentRequired				,	"Payment Required"				},
								{	HTTP_StatusCode::HTTPSC_Forbidden					,	"Forbidden"						},
								{	HTTP_StatusCode::HTTPSC_NotFound					,	"Not Found"						},
								{	HTTP_StatusCode::HTTPSC_MethodNotAllowed			,	"Method Not Allowed"			},
								{	HTTP_StatusCode::HTTPSC_NotAcceptable				,	"Not Acceptable"				},
								{	HTTP_StatusCode::HTTPSC_ProxyAuthenticationRequired	,	"Proxy Authentication Required"	},
								{	HTTP_StatusCode::HTTPSC_RequestTimeout				,	"Request Timeout"				},
								{	HTTP_StatusCode::HTTPSC_Conflict					,	"Conflict"						},
								{	HTTP_StatusCode::HTTPSC_Gone						,	"Gone"							},
								{	HTTP_StatusCode::HTTPSC_LengthRequired				,	"Length Required"				},
								{	HTTP_StatusCode::HTTPSC_PreconditionFailed			,	"Precondition Failed"			},
								{	HTTP_StatusCode::HTTPSC_RequestEntityTooLarge		,	"Request Entity Too Large"		},
								{	HTTP_StatusCode::HTTPSC_RequestURITooLong			,	"Request-URI Too Long"			},
								{	HTTP_StatusCode::HTTPSC_UnsupportedMediaType		,	"Unsupported Media Type"		},
								{	HTTP_StatusCode::HTTPSC_RequestRangeNotSatisfiable	,	"Request Range Not Satisfiable"	},
								{	HTTP_StatusCode::HTTPSC_ExpectationFailed			,	"Expectation Failed"			},
								{	HTTP_StatusCode::HTTPSC_InternalServerError			,	"Internal Server Error"			},
								{	HTTP_StatusCode::HTTPSC_NotImplemented				,	"Not Implemented"				},
								{	HTTP_StatusCode::HTTPSC_BadGateway					,	"Bad Gateway"					},
								{	HTTP_StatusCode::HTTPSC_ServiceUnavailable			,	"Service Unavailable"			},
								{	HTTP_StatusCode::HTTPSC_GatewayTimeout				,	"Gateway Timeout"				},
								{	HTTP_StatusCode::HTTPSC_HTTPVersionNotSupported		,	"HTTP Version Not Supported"	},
							};

const char * GetResponseText(const HTTPSC _resp)
{
	for(auto sts : STS)
		if(sts.Code == _resp)
			return(sts.Text);

	return("Response Code Not Found");
}

DWORD WINAPI GoHandleRequests(_In_ LPVOID lpParameter)
{
	while(ProcessRequest(*((SOCKET *) lpParameter)));
	return(0);
}

void GoHandleClient(SOCKET _client)
{
	LaunchThread(GoHandleRequests,&_client);
}

void HTTP_Message::Parse(const char * _buffer)
{
STRING	tmp;
vSTRING	vstr;
int		start,end,idx;

	start = 0;
//
	for(end=start;_buffer[end] && EOL(_buffer[end]) == false;end++);

	StatusLine = "";
	StatusLine.Grow((end-start)+1);
	memcpy(StatusLine.String,&_buffer[start],end-start);
	StatusLine.String[end-start] = 0x00;

	for(start=end;_buffer[start] && EOL(_buffer[start]);start++);
//
	HeaderFields.clear();
	for(idx=0;_buffer[start];idx++)
	{
		for(end=start;_buffer[end] && EOL(_buffer[end]) == false;end++);
		if(_buffer[start])
		{
			tmp = "";
			tmp.Grow((end-start)+1);
			memcpy(tmp.String,&_buffer[start],end-start);
			tmp.String[end-start] = 0x00;
			HeaderFields.push_back(tmp);

			for(start=end;_buffer[start] && EOL(_buffer[start]);start++);
		}
	}

	MessageBody.clear();	
	for(idx=0;_buffer[start];idx++)
	{
		for(end=start;_buffer[end] && EOL(_buffer[end]) == false;end++);
		if(_buffer[start])
		{
			tmp = "";
			tmp.Grow((end-start)+1);
			memcpy(tmp.String,&_buffer[start],end-start);
			tmp.String[end-start] = 0x00;
			MessageBody.push_back(tmp);
		}
	}
}

bool ProcessRequest(SOCKET _Client)
{
char *				rsc_buffer = nullptr;
char				net_buffer[NET_BUFFER_SIZE];
HTTPSC				resp;
const char *		content;
vSTRING				parms;
DWORD				size;
int					idx,count,hdr_bytes;
HTTP_Message		msg;

	recv(_Client,net_buffer,NET_BUFFER_SIZE,0);

	msg.Parse(net_buffer);

	count = msg.StatusLine.Split(' ',parms);
	if(count >= 3 && parms[count-1] == Protocol)
	{
		for(idx=0;idx<REQ_TYPES && parms[0] != rname[idx].Name;idx++);
		switch(idx)
		{
		case RT_OPTIONS :
			break;
		case RT_GET :
			printf("Received GET request for %s\n",parms[1].String);
			resp = HTTP_StatusCode::HTTPSC_OK;
			if((size = LoadResourceFile(parms[1],&rsc_buffer,&content)) == 0)
			{
				resp = HTTP_StatusCode::HTTPSC_NotFound;
				rsc_buffer = new char[NET_BUFFER_SIZE];
				sprintf_s(rsc_buffer,NET_BUFFER_SIZE,"<html><body><H1>Error %d (%s)</H1></body></html>",resp,GetResponseText(resp));
			}
			hdr_bytes = sprintf_s(net_buffer,NET_BUFFER_SIZE,"%s %d %s\nContent-length: %d\nContent-Type: %s\n\n",Protocol,resp,GetResponseText(resp),size,content);
			send(_Client,(const char *) &net_buffer,hdr_bytes,0);
			send(_Client,(const char *) rsc_buffer,size,0);
			delete [] rsc_buffer;
			break;
		case RT_HEAD :
		case RT_POST :
		case RT_PUT :
		case RT_DELETE :
		case RT_TRACE :
		case RT_CONNECT :
		default :
			break;
		}
	}
	return(false);
}

void GoServeThePeople(void)
{
SOCKET				sock,client;
int                 bTrue = true;
struct sockaddr_in  server_addr,client_addr;
int                 sin_size;
		 
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Socket");
		exit(1);
	}
 
	if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(const char *) &bTrue,sizeof(int)) == -1)
	{
		perror("Setsockopt");
		exit(1);
	}
		 
	server_addr.sin_family = AF_INET;         
	server_addr.sin_port = htons(SERVICE_PORT);     
	server_addr.sin_addr.s_addr = INADDR_ANY; 

	memset(&(server_addr.sin_zero),0,8);
	if(bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))== -1) 
	{
		perror("Unable to bind");
		exit(1);
	}
 
	if(listen(sock, 5) == -1) 
	{
		perror("Listen");
		exit(1);
	}
   
	printf("Progressive WebServer waiting on port %d\n\n",SERVICE_PORT);
	fflush(stdout);

	sin_size = sizeof(struct sockaddr_in);

	while((client = accept(sock,(struct sockaddr *) &client_addr,&sin_size)) != INVALID_SOCKET)
	{
//		printf("\nConnection from ( %s , %d )",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
		GoHandleClient(client);
	}

	closesocket(sock);
}

int main(void)
{
	for(Running=true;Running;)
	{
		GetReadyForOffice();
		GoServeThePeople();
		CleanUpTheMess();
	}

	return(0);
}
