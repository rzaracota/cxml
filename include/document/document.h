#ifndef _document_header_
#define _document_header_

CXMLDocument * CXMLDocument_Create(const char * resource);

CXMLDocument * CXMLDocument_AddAttribute(CXMLDocument * document,
					 char * attribute, char * value);


CXMLDocument * CXMLDocument_Destroy(CXMLDocument * document);

Node * CXMLDocument_Node_Add_Node(Node * node, Node * new_node);
Node * CXMLDocument_Node_Add_Attribute(Node * node, Attribute * attribute);

Node * cxml_get_node(Node * nodes, const unsigned int node_count,
		     const char * const node_name);
char * cxml_get_attribute_value(const Attribute * attributes,
				const unsigned int attribute_count,
				const char * const attribute_name);

void cxml_display_node(const Node * const node);
void cxml_display_attribute(const Attribute * attribute);

#endif
