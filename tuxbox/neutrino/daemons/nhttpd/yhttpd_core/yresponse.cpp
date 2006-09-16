//=============================================================================
// YHTTPD
// Response
//=============================================================================

// c
#include <cstdarg>
#include <cstdio>
// c++
#include <string.h>
// system
#include <fcntl.h>
#include <sys/socket.h>
// yhttpd
#include "yconfig.h"
#include "yhttpd.h"
#include "ytypes_globals.h"
#include "ylogging.h"
#include "ywebserver.h"
#include "yconnection.h"
#include "helper.h"
#include "yhook.h"

#ifdef Y_CONFIG_HAVE_SENDFILE
#include <sys/sendfile.h>
#endif

//=============================================================================
// Constructor & Destructor
//=============================================================================
CWebserverResponse::CWebserverResponse(CWebserver *pWebserver)
{
	Webserver = pWebserver;
	CWebserverResponse();
}
//-----------------------------------------------------------------------------
CWebserverResponse::CWebserverResponse()
{
	obj_content_len = 0;
	obj_last_modified = (time_t)0;
}

//=============================================================================
// Main Dispacher for Response
// To understand HOOKS reade yhook.cpp Comments!!!
//-----------------------------------------------------------------------------
// RFC 2616 / 6 Response
//
//	After receiving and interpreting a request message, a server responds
//	with an HTTP response message.
//	Response       =Status-Line		; generated by SendHeader
//			*(( general-header	; generated by SendHeader
//			 | response-header	; generated by SendHeader
//			 | entity-header ) CRLF); generated by SendHeader
//			 CRLF			; generated by SendHeader
//			 [ message-body ]	; by HOOK Handling Loop or Sendfile
//=============================================================================
bool CWebserverResponse::SendResponse()
{
	log_level_printf(9,"SendResponse Start\n");
	//--------------------------------------------------------------
	// HOOK Handling Loop [ response hook ]
	// Dynamic Data Creation 
	//--------------------------------------------------------------
	do
	{
		if(Connection->RequestCanceled)
			return false;
		Connection->HookHandler.session_init(Connection->Request.ParameterList, Connection->Request.UrlData, 
			(Connection->Request.HeaderList), (Cyhttpd::ConfigList), Connection->Method);
		Connection->HookHandler.Hooks_SendResponse();
		if((Connection->HookHandler.status == HANDLED_READY)||(Connection->HookHandler.status == HANDLED_CONTINUE))
		{
			log_level_printf(1,"Response Hook Output. Status:d\n", Connection->HookHandler.status);
			// Add production data to HTML-Output
			if(Connection->HookHandler.ResponseMimeType == "text/html")
			{
				Connection->HookHandler.printf("<!-- request time: 0,%ld sec production time: 0,%ld -->\n",
					Connection->enlapsed_request, Connection->GetEnlapsedResponseTime());
			}
			obj_content_len = Connection->HookHandler.yresult.length(); 
			SendHeader(Connection->HookHandler.httpStatus, false, Connection->HookHandler.ResponseMimeType);
			Write(Connection->HookHandler.yresult);
			if(Connection->HookHandler.status != HANDLED_CONTINUE)
				return true;
		}
		else if(Connection->HookHandler.status == HANDLED_ERROR)
		{
			log_level_printf(1,"Response Hook found but Error\n");
			SendHeader(Connection->HookHandler.httpStatus, false, Connection->HookHandler.ResponseMimeType);
			Write(Connection->HookHandler.yresult);
			return false;
		}
		else if(Connection->HookHandler.status == HANDLED_ABORT)
			return false;
		// URL has new value. Analyze new URL for SendFile
		else if(Connection->HookHandler.status == HANDLED_SENDFILE ||
			Connection->HookHandler.status == HANDLED_REWRITE)
				Connection->Request.analyzeURL(Connection->HookHandler.NewURL);
		if(Connection->HookHandler.status == HANDLED_REDIRECTION)
		{
			SendObjectMoved(Connection->HookHandler.NewURL);
			return false;
		}
	}
	while(Connection->HookHandler.status == HANDLED_REWRITE);
	//--------------------------------------------------------------

#ifdef Y_CONFIG_USE_HOSTEDWEB
	// for hosted webs: rewrite URL
	std::string _hosted="/hosted/";
	if((Connection->Request.UrlData["path"]).compare(0,_hosted.length(),"/hosted/") == 0)		// hosted Web ?
		Connection->Request.UrlData["path"]=Cyhttpd::ConfigList["HostedDocumentRoot"]
			+(Connection->Request.UrlData["path"]).substr(_hosted.length()-1);
#endif //Y_CONFIG_USE_HOSTEDWEB

	// no handler found, try send static file
	return SendFile(Connection->Request.UrlData["path"],Connection->Request.UrlData["filename"]);	//normal file
}

//=============================================================================
// Send Headers
//-----------------------------------------------------------------------------
// RFC 2616 / 6 Response (Header)
//
//   The first line of a Response message is the Status-Line, consisting
//   of the protocol version followed by a numeric status code and its
//   associated textual phrase, with each element separated by SP
//   characters. No CR or LF is allowed except in the final CRLF sequence.
//
//	Response       =Status-Line		; generated by SendHeader
//			*(( general-header
//			 | response-header
//			 | entity-header ) CRLF)
//			 CRLF			; generated by SendHeader
//			 [ message-body ]	; by HOOK Handling Loop or Sendfile
//
//	Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF ; generated by SendHeader
//
//	general-header = Cache-Control             ; not implemented
//			| Connection               ; implemented
//			| Date                     ; implemented
//			| Pragma                   ; not implemented
//			| Trailer                  ; not implemented
//			| Transfer-Encoding        ; not implemented
//			| Upgrade                  ; not implemented
//			| Via                      ; not implemented
//			| Warning                  ; not implemented
//
//       response-header = Accept-Ranges          ; not implemented
//                      | Age                     ; not implemented
//                      | ETag                    ; not implemented
//                      | Location                ; implemented (redirection / Object moved)
//                      | Proxy-Authenticate      ; not implemented
//                      | Retry-After             ; not implemented
//                      | Server                  ; implemented
//                      | Vary                    ; not implemented
//                      | WWW-Authenticate        ; implemented (by mod_auth and SendHeader)
//                       
//       entity-header  = Allow                    ; not implemented
//                      | Content-Encoding         ; not implemented
//                      | Content-Language         ; not implemented
//                      | Content-Length           ; implemented
//                      | Content-Location         ; not implemented
//                      | Content-MD5              ; not implemented
//                      | Content-Range            ; not implemented
//                      | Content-Type             ; implemented
//                      | Expires                  ; not implemented
//                      | Last-Modified            ; implemented for static files
//                      | extension-header
//
//       extension-header = message-header                       
//=============================================================================
void CWebserverResponse::SendHeader(HttpResponseType responseType, bool cache, std::string ContentType)
{
	const char *responseString = "";
	const char *infoString = 0;

	// get Info Index
	for (unsigned int i = 0;i < (sizeof(httpResponseNames)/sizeof(httpResponseNames[0])); i++)
		if (httpResponseNames[i].type == responseType)
		{
			responseString = httpResponseNames[i].name;
			infoString = httpResponseNames[i].info;
			break;
		}
	// print Status-line
	printf("HTTP/1.1 %d %s\r\nContent-Type: %s\r\n",responseType, responseString, ContentType.c_str());

	switch (responseType)
	{
		case HTTP_UNAUTHORIZED:
//			WriteLn("HTTP/1.0 401 Unauthorized");
			Write("WWW-Authenticate: Basic realm=\"");
			WriteLn(AUTH_NAME_MSG);
			break;
			
		default:
			// Status HTTP_*_TEMPORARILY (redirection)
			if(redirectURI != "")
				printf("Location: %s\r\n",redirectURI.c_str());
			// cache
			if(!cache)
				WriteLn("Cache-Control: no-cache");
			WriteLn( "Server: " WEBSERVERNAME );
			// actual date
			time_t timer = time(0);
			char timeStr[80];
			strftime(timeStr, sizeof(timeStr), RFC1123FMT, gmtime(&timer));
			printf("Date: %s\r\n", timeStr);
			// connection type
			WriteLn("Connection: keep-alive");
			//TODO: Content_length at errors
			//TODO:FD WriteLn("Connection: close");
			// content-len, last-modified
			if(responseType == HTTP_NOT_MODIFIED ||responseType == HTTP_NOT_FOUND)
				WriteLn("Content-Length: 0");
			else if(obj_content_len >0)
			{    
				strftime(timeStr, sizeof(timeStr), RFC1123FMT, gmtime(&obj_last_modified));
				printf("Last-Modified: %s\r\nContent-Length: %ld\r\n", timeStr, obj_content_len);
			}			
			WriteLn("");	// End of Header
			break;
	}

	// Body
	if (Connection->Method != M_HEAD)
        switch (responseType)
        {
		case HTTP_OK:
		case HTTP_NOT_MODIFIED:
		case HTTP_CONTINUE:
		case HTTP_ACCEPTED:
		case HTTP_NO_CONTENT:
		case HTTP_NOT_FOUND:
			break;

		case HTTP_MOVED_TEMPORARILY:
		case HTTP_MOVED_PERMANENTLY:
			Write("<html><head><title>Object moved</title></head><body>");
			printf("302 : Object moved.<br/>If you dont get redirected click <a href=\"%s\">here</a></body></html>\n",redirectURI.c_str());	
                	break;

		default:
//#ifdef Y_CONFIG_HAVE_HTTPD_ERRORPAGE
//// IDEA: Add Config for ErrorPage like HTTPD_ERROR_PAGE = "/Y_Error.yhtm"
//			Connection->Request.Filename = HTTPD_ERRORPAGE;
//			Webserver->yParser->ParseAndSendFile(Connection);
//#else
//            		if(ContentType == "text/html")
//	                	SendHTMLHeader(responseString);
//			printf("%d : %s\r\n",responseType, infoString);
//            		if(ContentType == "text/html")
//	                	SendHTMLFooter();
//#endif
                	break;
        }
	Connection->HttpStatus = responseType;
}

//=============================================================================
// Output 
//=============================================================================
//-----------------------------------------------------------------------------
// BASIC Send over Socket for Strings (char*)
//-----------------------------------------------------------------------------
bool CWebserverResponse::WriteData(char const * data, long length)
{
	if(Connection->RequestCanceled)
		return false;
	if(Connection->sock->Send(data, length) == -1)
	{
		perror("request canceled\n");
		Connection->RequestCanceled = true;
		return false;
	}
	else
		return true;
}
//-----------------------------------------------------------------------------
#define bufferlen 4*1024
void CWebserverResponse::printf ( const char *fmt, ... )
{
	char buffer[bufferlen];
	va_list arglist;
	va_start( arglist, fmt );
	vsnprintf( buffer, bufferlen, fmt, arglist );
	va_end(arglist);
	Write(buffer);
}
//-----------------------------------------------------------------------------
bool CWebserverResponse::Write(char const *text)
{
	return WriteData(text, strlen(text));
}
//-----------------------------------------------------------------------------
bool CWebserverResponse::WriteLn(char const *text)
{
	if(!WriteData(text, strlen(text)))
		return false;
	return WriteData("\r\n",2);
}

//=============================================================================
// Send HTMLs
//=============================================================================
void CWebserverResponse::SendHTMLHeader(std::string Titel)
{
	WriteLn("<html>\n<head><title>" + Titel + "</title>");
	WriteLn("<meta http-equiv=\"cache-control\" content=\"no-cache\">");
	WriteLn("<meta http-equiv=\"expires\" content=\"0\"></head>\n<body>");
}
//-----------------------------------------------------------------------------
void CWebserverResponse::SendHTMLFooter(void)
{
	WriteLn("</body></html>");
}
//-----------------------------------------------------------------------------
void CWebserverResponse::SendObjectMoved(std::string URI)
{
	redirectURI = URI;
	SendHeader(HTTP_MOVED_TEMPORARILY,true, "text/html");
}

//=============================================================================
// Send File (functions)
//=============================================================================
//-----------------------------------------------------------------------------
// Send a File (main) with given path and filename.
// It procuced a Response-Header (SendHeader) and sends the file if exists.
// It supports Client caching mechanism "If-Modified-Since".
//-----------------------------------------------------------------------------
// RFC 2616 / 14.25 If-Modified-Since
//
//   The If-Modified-Since request-header field is used with a method to
//   make it conditional: if the requested variant has not been modified
//   since the time specified in this field, an entity will not be
//   returned from the server; instead, a 304 (not modified) response will
//   be returned without any message-body.
//
//       If-Modified-Since = "If-Modified-Since" ":" HTTP-date
//   An example of the field is:
//
//       If-Modified-Since: Sat, 29 Oct 1994 19:43:31 GMT
//
//   A GET method with an If-Modified-Since header and no Range header
//   requests that the identified entity be transferred only if it has
//   been modified since the date given by the If-Modified-Since header.
//   The algorithm for determining this includes the following cases:
//
//      a) If the request would normally result in anything other than a
//         200 (OK) status, or if the passed If-Modified-Since date is
//         invalid, the response is exactly the same as for a normal GET.
//         A date which is later than the server's current time is
//         invalid.
//
//      b) If the variant has been modified since the If-Modified-Since
//         date, the response is exactly the same as for a normal GET.
//
//      c) If the variant has not been modified since a valid If-
//         Modified-Since date, the server SHOULD return a 304 (Not
//         Modified) response.
//         
//  yjogol: ASSUMPTION Date-Format is ONLY RFC 1123 compatible!       
//-----------------------------------------------------------------------------
bool CWebserverResponse::SendFile(const std::string path,const std::string filename)
{
	if(Connection->RequestCanceled)
		return false;

	int filed;
	log_level_printf(9,"<SendFile>: File:%s Path:%s\n", filename.c_str(), path.c_str() );  
	if( (filed = OpenFile(path, filename) ) != -1 ) //can access file?
	{
		// check If-Modified-Since
		time_t if_modified_since = (time_t)-1;
#ifndef Y_UPDATE_BETA		
		if(Connection->Request.HeaderList["If-Modified-Since"] != "")
		{
			struct tm mod;
			if(strptime(Connection->Request.HeaderList["If-Modified-Since"].c_str(), RFC1123FMT, &mod) != NULL)
			{
				mod.tm_isdst = 0; // daylight saving flag! 
				if_modified_since = mktime(&mod);
			}
		}
#endif
		// normalize obj_last_modified to GMT
		struct tm *tmp = gmtime(&obj_last_modified);
		time_t obj_last_modified_gmt = mktime(tmp);
		bool modified = (if_modified_since == (time_t)-1) || (if_modified_since < obj_last_modified_gmt);

		// Send normal or not-modified header
		if(modified)
			SendHeader(HTTP_OK, true, GetContentType(Connection->Request.UrlData["fileext"]));
		else
			SendHeader(HTTP_NOT_MODIFIED, true, GetContentType(Connection->Request.UrlData["fileext"]));

		// senf file if modified
		if((Connection->Method != M_HEAD) && modified)
			Connection->sock->SendFile(filed);
		close(filed);
		return true;
	}
	else
	{
		aprintf("<SendFile>: File not found\n");
		SendError(HTTP_NOT_FOUND);
		return false;
	}
}
//-----------------------------------------------------------------------------
// Send File: Build Filename
// First Look at PublicDocumentRoot than PrivateDocumentRoot than pure path
//-----------------------------------------------------------------------------
std::string CWebserverResponse::GetFileName(std::string path, std::string filename)
{
	std::string tmpfilename;
	if(path[path.length()-1] != '/')
		tmpfilename = path + "/" + filename;
	else
		tmpfilename = path + filename;

	if( access(std::string(Cyhttpd::ConfigList["PublicDocumentRoot"] + tmpfilename).c_str(),4) == 0)
		tmpfilename = Cyhttpd::ConfigList["PublicDocumentRoot"] + tmpfilename;
	else if(access(std::string(Cyhttpd::ConfigList["PrivatDocumentRoot"] + tmpfilename).c_str(),4) == 0)
		tmpfilename = Cyhttpd::ConfigList["PrivatDocumentRoot"] + tmpfilename;
	else if(access(tmpfilename.c_str(),4) == 0)
		;
	else
	{
		return "";
	}
	return tmpfilename;
}
//-----------------------------------------------------------------------------
// Send File: Open File and check file type
//-----------------------------------------------------------------------------
int CWebserverResponse::OpenFile(std::string path, std::string filename)
{
	struct stat statbuf;
	int  fd= -1;
	std::string tmpstring;

	tmpstring = GetFileName(path, filename);
	if(tmpstring.length() > 0)
	{
		fd = open( tmpstring.c_str(), O_RDONLY );
		if (fd<=0)
		{
			aprintf("cannot open file %s: ", filename.c_str());
			dperror("");
		}
		// It is a regular file?
		fstat(fd,&statbuf);
		if (!S_ISREG(statbuf.st_mode)) {
			close(fd);
			fd = -1;
		}
		// get file size and modify date
		obj_content_len = statbuf.st_size;
		obj_last_modified = statbuf.st_mtime;
	}
	return fd;
}
//-----------------------------------------------------------------------------
// Send File: Determine MIME-Type fro File-Extention
//-----------------------------------------------------------------------------
std::string CWebserverResponse::GetContentType(std::string ext)
{
	std::string ctype = "text/plain";
	ext = string_tolower(ext);
	for (unsigned int i = 0;i < (sizeof(MimeFileExtensions)/sizeof(MimeFileExtensions[0])); i++)
		if (MimeFileExtensions[i].fileext == ext)
		{
			ctype = MimeFileExtensions[i].mime;
			break;
		}	
	return ctype;
}
