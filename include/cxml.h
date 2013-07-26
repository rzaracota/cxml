#ifndef _cxml_header_
#define _cxml_header_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>

void * safe_free(void * pointer);

typedef struct _Declaration {
  char * version;
  char * language;
  char * encoding;
} Declaration;

typedef struct _Attribute {
  char * identifier;
  char * value;
} Attribute;

typedef struct _Node {
  char * identifier;
  unsigned int attribute_count;
  unsigned int node_count;
  Attribute * attributes;
  struct _Node * nodes;
  char * pcdata;
} Node;

typedef struct _CXMLDocument {
  char * resource;
  Declaration declaration;
  unsigned int attribute_count;
  unsigned int node_count;
  Attribute * attributes;
  union {
    struct _Node * nodes;
    char * pcdata;
  };
} CXMLDocument;

#include "document/document.h"

#include "load/load.h"

#include "save/save.h"

#endif
