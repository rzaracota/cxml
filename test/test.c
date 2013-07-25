#include "cxml.h"

#include "document/document.h"

#include "load/load.h"

static void display_document(const CXMLDocument * document);

int main(int argc, char * argv[]) {
  CXMLDocument * document = CXMLDocument_Create("test_document");

  display_document(document);

  document = CXMLDocument_Destroy(document);

  document = CXMLLoad("test/xml/scene.xml");

  display_document(document);

  return 0;
}

static void display_attribute(const Attribute * attribute) {
  printf("identifier: %s\n", attribute->identifier);
  printf("value: %s\n", attribute->value);
}

static void display_attributes(const Node * node) {
  printf("--attributes--\n");

  for (int i = 0; i < node->attribute_count; i++) {
    printf("[%d]:\n", i);

    display_attribute(node->attributes + i);
  }
}

static void display_declaration(const Declaration * declaration) {
  printf("--Declaration--\n");

  printf("version: %s\n", declaration->version);
  printf("encoding: %s\n", declaration->encoding);
  printf("language: %s\n", declaration->language);
}

static void display_document(const CXMLDocument * document) {
  printf("--CXMLDocument--\n");

  if (document == 0) {
    printf("Invalid document.\n");

    return;
  }

  printf("resource: %s\n", document->resource);

  printf("attribute_count: %d\n", document->attribute_count);

  printf("node_count: %d\n", document->node_count);
  printf("nodes: %p\n", document->nodes);

  display_declaration(&document->declaration);

  if (document->attribute_count > 0) {
    Node node = { .attribute_count = document->attribute_count,
		  .attributes = document->attributes };

    display_attributes(&node);
  }
}
