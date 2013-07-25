#include "cxml.h"

#include "document/document.h"

CXMLDocument * CXMLDocument_Create(const char * resource) {
  CXMLDocument * document = (CXMLDocument *)malloc(sizeof (CXMLDocument));

  if (resource == 0) {
    return document;
  }

  document->resource = (char *)malloc(sizeof (char) * (strlen(resource) + 1));

  strcpy(document->resource, resource);

  document->attribute_count = 0;
  document->node_count = 0;

  document->attributes = 0;

  document->nodes = 0;

  return document;
}

CXMLDocument * CXMLDocument_AddAttribute(CXMLDocument * document,
					 char * attribute, char * value) {
  if (attribute == 0 || value == 0) {
    printf("##invalid attribute specified.##\n");

    return document;
  }

  Attribute * ab = (Attribute *)malloc(sizeof (Attribute) *
				       (document->attribute_count + 1));

  if (document->attribute_count > 0) {
    if (document->attributes == 0) {
      printf("##document attributes misaligned.##\n");

      return document;
    }

    memcpy(ab, document->attributes, sizeof (Attribute) *
	   document->attribute_count);
  }

  int iter = document->attribute_count;

  char * string = attribute;
  int len = strlen(string) + 1;

  ab[iter].identifier = (char *)malloc(sizeof (char) * len);
  strcpy(ab[iter].identifier, string);

  string = value;

  len = strlen(string) + 1;

  ab[iter].value = (char *)malloc(sizeof (char) * len);
  strcpy(ab[iter].value, string);

  document->attributes = ab;

  return document;
}

static Attribute * destroy_attribute(Attribute * attribute) {
  return 0;
}

static Attribute * destroy_attributes(Attribute * attributes) {
  return 0;
}

static Node * destroy_node(Node * node) {
  return 0;
}

static Node * destroy_nodes(Node * node) {
  return 0;
}

static void destroy_declaration(Declaration * declaration) {
  if (declaration == 0) {
    return;
  }

  if (declaration->version != 0) {
    safe_free(declaration->version);
  }

  if (declaration->language != 0) {
    safe_free(declaration->language);
  }

  if (declaration->encoding != 0) {
    safe_free(declaration->encoding);
  }
}

static CXMLDocument * confirm_destruction(CXMLDocument * document) {
  if (document->resource != 0 || document->nodes != 0 ||
      document->attributes != 0 || document->declaration.version != 0 ||
      document->declaration.language != 0 ||
      document->declaration.encoding != 0) {
    printf("##document not completely destroyed##\n");

    printf("%p %p %p %p %p %p\n", document->resource, document->nodes,
	   document->attributes, document->declaration.version,
	   document->declaration.language,
	   document->declaration.encoding);
  } else {
    document = safe_free(document);
  }

  return document;
}

static CXMLDocument * destroy_document(CXMLDocument * document) {
  if (document == 0) {
    return document;
  }

  if (document->nodes != 0) {
    if (document->node_count == 0) {
      document->nodes = safe_free(document->nodes);
    } else {
      document->nodes = destroy_nodes(document->nodes);
    }
  }

  if (document->attributes != 0) {
    if (document->attribute_count == 0) {
      document->attributes = safe_free(document->attributes);
    } else {
      document->attributes = destroy_attributes(document->attributes);
    }
  }

  document->resource = safe_free(document->resource);

  destroy_declaration(&document->declaration);

  return confirm_destruction(document);
}

CXMLDocument * CXMLDocument_Destroy(CXMLDocument * document) {
  return destroy_document(document);
}
