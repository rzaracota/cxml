#include "cxml.h"

static const char * error_msg = "cxml-save";

static void indent(FILE * fp, unsigned int depth);

static void write_attribute(const Attribute * attribute, FILE * fp);
static void write_attributes(const Node * const node, FILE * fp);

static void write_element_footer(const Node * node, FILE * fp);
static void write_element_header(const Node * node, FILE * fp);

static void write_node(const Node * node, FILE * fp);
static void write_document_nodes(const CXMLDocument * document, FILE * fp);

static void write_declaration(Declaration * const declaration,
			      FILE * fp);

void CXMLDocument_Save(CXMLDocument * document,
		       const char * const filename) {
  FILE * fp = stdout;

  if (filename != 0) {
    fp = fopen(filename, "w");
  }

  if (fp == 0) {
    printf("##%s: failed to open request stream.##\n",
	   error_msg);

    return;
  }

  write_declaration(&document->declaration, fp);

  write_document_nodes(document, fp);

  fclose(fp);
}

static void indent(FILE * fp, unsigned int depth) {
  for (int t = 0; t < depth; t++) {
    fprintf(fp, " ");
  }
}

static void write_attribute(const Attribute * attribute, FILE * fp) {
  fprintf(fp, "%s=\"%s\"", attribute->identifier, attribute->value);
}

static void write_attributes(const Node * const node, FILE * fp) {
  for (int i = 0; i < node->attribute_count; i++) {
    fprintf(fp, " ");

    write_attribute(node->attributes + i, fp);
  }
}

static void write_element_footer(const Node * node, FILE * fp) {
  if (node->node_count > 0) {
    fprintf(fp, "</%s>\n", node->identifier);
  } else if (node->pcdata != 0) {
    fprintf(fp, "</%s>\n", node->identifier);
  }
}

static void write_element_header(const Node * node, FILE * fp) {
  fprintf(fp, "<%s", node->identifier);

  if (node->attribute_count > 0) {
    write_attributes(node, fp);
  }

  if (node->node_count == 0 && node->pcdata == 0) {
    fprintf(fp, " />\n");
  } else if (node->node_count > 0) {
    fprintf(fp, ">\n");
  } else {
    fprintf(fp, ">");
  }
}

static void write_node(const Node * node, FILE * fp) {
  static int depth = 0;

  if (node == 0) {
    printf("##%s: null node##\n", error_msg);

    return;
  }

  indent(fp, depth);

  write_element_header(node, fp);

  if (node->node_count > 0) {
    for (int i = 0; i < node->node_count; i++) {
	depth++;

	write_node(node->nodes + i, fp);
    }
  } else if (node->pcdata != 0) {
    fprintf(fp, "%s", node->pcdata);
  }

  if (node->node_count > 0) {
    indent(fp, depth);
  }

  write_element_footer(node, fp);

  if (depth > 0) {
    depth--;
  }
}

static void write_document_nodes(const CXMLDocument * document, FILE * fp) {
  if (document == 0) {
    printf("##%s: null document##\n", error_msg);

    return;
  }

  Node node;

  node.identifier = document->resource;

  node.attribute_count = document->attribute_count;
  node.attributes = document->attributes;

  node.node_count = document->node_count;
  node.nodes = document->nodes;

  node.pcdata = document->pcdata;

  write_node(&node, fp);
}

static void write_declaration(Declaration * const declaration,
			      FILE * fp) {
  if (declaration == 0) {
    printf("##%s: null declaration##\n", error_msg);

    return;
  }

  if (declaration->version == 0) {
    declaration->version = "1.0";
  }

  if (declaration->language == 0) {
    declaration->language = "english";
  }

  if (declaration->encoding == 0) {
    declaration->encoding = "utf-8";
  }

  fprintf(fp, "<?xml version=\"%s\" language=\"%s\" encoding=\"%s\" ?>\n",
	  declaration->version, declaration->language,
	  declaration->encoding);
}
