#ifndef _document_header_
#define _document_header_

CXMLDocument * CXMLDocument_Create(const char * resource);

CXMLDocument * CXMLDocument_AddAttribute(CXMLDocument * document,
					 char * attribute, char * value);


CXMLDocument * CXMLDocument_Destroy(CXMLDocument * document);

#endif
