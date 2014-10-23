#ifndef _bgHttpAppWeb_h_
#define _bgHttpAppWeb_h_

#include "bgTcpSocketClientServer.h"


typedef struct httpAppWeb_typ{
    /*Note: following lists require external ("allocated") memory for storing list of ptr-s!*/
    bgStringListEx_typ listContentLines;   /*list of lines (char*) in the HTTP request content area (if any)*/    


}httpAppWeb_typ;


void bgWebServerApplication(bgTcpSocketSession_typ* pSession);
void bgWebClientApplication(bgTcpSocketSession_typ* pSession);

#endif
