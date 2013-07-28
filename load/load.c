#include "cxml.h"

enum { true = 1, false = 0 };

static char * buffer = 0;

typedef enum _TokenType { SPACE = ' ', NEWLINE = '\n', TAB = '\t',
			  LEFT_ALLIGATOR = '<', RIGHT_ALLIGATOR = '>',
			  FORWARD_SLASH = '/', DOUBLE_QUOTE = '\"',
			  SINGLE_QUOTE = '\'', EQUALS = '='
                        } TokenType;		          

static unsigned int whitespace(TokenType token) {
  if (token == SPACE || token == NEWLINE || token == TAB) {
    return true;
  }

  return false;
}

static unsigned int comment(char * const beg) {
  if (*(beg + 1) == '!' &&
      *(beg + 2) == '-' &&
      *(beg + 3) == '-') {
    return true;
  }

  return false;
}

static void skip_comment(char ** const iterator) {
  int length = 0;

  (*iterator) += 4;

  while (whitespace(*(*iterator))) {
    (*iterator)++;
  }

  char * beg = *iterator;

  while (*(*iterator) != 0) {
    if (*(*iterator) == RIGHT_ALLIGATOR &&
	*(*iterator - 1) == '-' &&
	*(*iterator - 2) == '-') {
      (*iterator)++;

      char * temp = (char *)malloc(sizeof (char) * (length - 2));

      strncpy(temp, beg, length - 4);

      temp[length - 2] = '\0';

      if (temp != 0) {
	free(temp);

	temp = 0;
      }

      break;
    }

    length++;

    (*iterator)++;
  } 
}

void skip_wac(char ** iterator) {
  while (**iterator != 0) {
    if (comment(*iterator)) {
      skip_comment(&(*iterator));
    }

    if (!whitespace(**iterator)) {
      break;
    }

    (*iterator)++;
  }
}

static char * get_value(char ** const iterator) {
  char * value = 0, * iter = *iterator, * begin = *iterator;

  if (*begin != EQUALS) {
    printf("orphan attribute\n%s", *iterator);

    return 0;
  }

  begin++;

  if (*begin != SINGLE_QUOTE && *begin != DOUBLE_QUOTE) {
    printf("value missing quote.\nFound: %c\n", *begin);

    return 0;
  }

  char quote = *begin;

  begin++;

  int i = 0;

  for (; begin[i] != quote; i++) {
    if (begin[i] == SINGLE_QUOTE ||
	begin[i] == DOUBLE_QUOTE) {
      printf("Mismatched quote.\n");

      return 0;
    }
  }

  value = (char *)malloc(sizeof (char) * (i + 1));

  strncpy(value, begin, i);

  value[i] = '\0';

  (*iterator) += i + 1 + (begin - (*iterator));

  return value;
}

static char * get_identifier(char ** const iterator) {
  char * identifier = 0, * iter = *iterator, * begin = *iterator;

  int i = 0;

  while (whitespace(iter[i]) == false &&
	 iter[i] != FORWARD_SLASH &&
	 iter[i] != RIGHT_ALLIGATOR &&
	 iter[i] != SINGLE_QUOTE &&
	 iter[i] != DOUBLE_QUOTE &&
	 iter[i] != EQUALS &&
	 iter[i] != 0) {
    i++;
  }

  (*iterator) += i;

  identifier = (char *)malloc(sizeof (char) * (i + 1));

  if (identifier == 0) {
    printf("##memory allocation error.##\n");

    return identifier;
  }

  strncpy(identifier, begin, i);

  identifier[i] = '\0';

  return identifier;
}

static Node * parse_attributes(Node * node, char ** const iterator) {
  if (node == 0) {
    printf("##null node in parse_attributes.\n##");

    return 0;
  }

  Attribute * attributes = 0;

  char * begin = *iterator;

  if (**iterator == RIGHT_ALLIGATOR ||
      **iterator == LEFT_ALLIGATOR ||
      **iterator == '/') {
    printf("no attributes present\n");

    return node;
  } else if (whitespace(**iterator)) {
    while (whitespace(*(++(*iterator))));
  }

  char * identifier = 0;
  char * value = 0;

  while (**iterator != 0) {
    while (whitespace(*(*iterator))) {
      (*iterator)++;
    }

    if (**iterator == '/' || **iterator == RIGHT_ALLIGATOR ||
	**iterator == '?') {
      break;
    }

    identifier = get_identifier(&(*iterator));

    value = get_value(&(*iterator));

    if (identifier != 0) {
      Attribute attribute = { .identifier = identifier,
			      .value = value };

      node = CXMLDocument_Node_Add_Attribute(node,
	       &((Attribute) { identifier, value }));

      identifier = 0;

      value = 0;
    }
  }

  return node;
}

static void parse_declaration(CXMLDocument * document,
				    char ** iterator) {
  char * iter = *iterator, * begin = *iterator;

  if (document == 0) {
    printf("##null document##\n");

    return;
  }

  *iterator += 2;

  char * xml = get_identifier(iterator);

  if (strcmp(xml, "xml") != 0) {
    printf("##invalid declaration node##\n");

    while (**iterator != RIGHT_ALLIGATOR) {
      (*iterator)++;
    }
  }

  Node * node = create_node();

  if (node == 0) {
    printf("##failed to allocate memory for node##\n");

    return;
  }

  node = parse_attributes(node, iterator);

  for (int i = 0; i < node->attribute_count; i++) {
    char * id = node->attributes[i].identifier;
    char * value = node->attributes[i].value;

    if (strcmp("version", id) == 0) {
      document->declaration.version = value;
    } else if (strcmp("encoding", id) == 0) {
      document->declaration.encoding = value;
    } else if (strcmp("language", id) == 0 || strcmp("lang", id) == 0) {
      document->declaration.language = value;
    } else {
      printf("##invalid declaration attribute: [%s] %s##\n",
	     id, value);
    }
  }

  if (**iterator != '?' || *(*iterator + 1) != RIGHT_ALLIGATOR) {
    printf("##invalid declaration close##\n");

    while (**iterator != '>') {
      *iterator++;
    }
  } else {
    *iterator += 2;
  }

  safe_free(node);
}

static char * get_pcdata(char ** iterator) {
  char * pcdata = 0, * begin = *iterator;

  int length = 0;

  while (**iterator != 0 && **iterator != LEFT_ALLIGATOR) {
    (*iterator)++;
    length++;
  }

  if (length > 0) {
    pcdata = (char *)malloc(sizeof (char) * (length + 1));

    strncpy(pcdata, begin, length);

    pcdata[length] = '\0';
  }

  return pcdata;
}

static Node * parse_opening_element(char ** iterator) {
  if (**iterator != LEFT_ALLIGATOR) {
    return 0;
  }

  Node * node = create_node();

  (*iterator)++;

  char * identifier = get_identifier(iterator);

  if (identifier == 0) {
    safe_free(node);

    return 0;
  }

  node->identifier = (char *)malloc(sizeof (char) * (strlen(identifier) + 1));

  strcpy(node->identifier, identifier);

  if (**iterator == ' ') {
    node = parse_attributes(node, &(*iterator));
  }

  if (**iterator == '/') {
    (*iterator)++;
  }

  (*iterator)++;

  return node;
}

static Node * parse_closing_element(Node * node, char ** iterator) {
  if (node == 0) {
    return 0;
  }

  (*iterator)++;

  if (**iterator != '/') {
    return node;
  }

  (*iterator)++;

  char * identifier = get_identifier(iterator);

  if (**iterator != RIGHT_ALLIGATOR) {
    printf("##illformatted closing node.##\n");
  } else {
    (*iterator)++;
  }

  if (strcmp(node->identifier, identifier) != 0) {
    printf("##Mistmatched closing node.##\n");
  }

  return node;
}

static Node * parse_node(char ** iterator) {
  skip_wac(iterator);

  Node * node = 0;

  node = parse_opening_element(iterator);

  if (node == 0) {
    printf("##invalid node##\n");
  
    return 0;
  }

  if (*(*iterator - 1) != RIGHT_ALLIGATOR) {
    printf("##invalid opening element##\n%s", *iterator - 1);

    return 0;
  } else if (*(*iterator - 2) == '/') {
    return node;
  }

  while (*(*iterator) != 0) {
    skip_wac(iterator);

    if (*(*iterator) != LEFT_ALLIGATOR) {
      node->pcdata = get_pcdata(iterator);
    } else if (*(*iterator + 1) != '/') {
      Node * new_node = parse_node(iterator);

      if (new_node != 0) {
	node = CXMLDocument_Node_Add_Node(node, new_node);

	safe_free(new_node);
      }
    } else {
      parse_closing_element(node, iterator);

      break;
    }
  }

  return node;
}

static char * setup_buffer(const char * filename) {
  FILE * fp = fopen(filename, "r");

  if (fp == 0) {
    printf("setup_buffer: failed to open %s\n", filename);

    return 0;
  }

  fseek(fp, 0L, SEEK_END);

  int file_size = -1;

  file_size = ftell(fp);

  rewind(fp);

  printf("file size: %d\n", file_size);
  printf("position: %d\n", fseek(fp, 0L, SEEK_SET));

  buffer = malloc(sizeof (char) * file_size);

  if (buffer == 0) {
    printf("##memory allocation for buffer failed.##\n");

    return 0;
  }

  int read = fread(buffer, file_size, 1, fp);

  if (read != 1) {
    printf("##file reading error##\n");
  }

  fclose(fp);

  return buffer;
}

static char * hybridize_name(char * const resource, char * const identifier) {
  char * hybrid_name = 0;

  if (resource != 0 && identifier != 0) {
    hybrid_name = (char *)malloc(sizeof (char) *
				 (strlen(resource) +
				  (strlen(identifier) + 1 + 1)));
    strncpy(hybrid_name, resource, strlen(resource));
    hybrid_name[strlen(resource)] = '#';

    strcpy(hybrid_name + strlen(resource) + 1, identifier);
  } else if (identifier == 0){
    hybrid_name = resource;
  } else if (resource == 0) {
    hybrid_name = identifier;
  } else {
    printf("##resource and identifier were both null..##\n");

    hybrid_name = 0;
  }

  return hybrid_name;
}

static void parse_document(CXMLDocument * document) {
  char * iterator = buffer;

  if (iterator == 0) {
    printf("##invalid buffer##\n");

    return;
  }

  skip_wac(&iterator);

  if (*iterator == LEFT_ALLIGATOR && *(iterator + 1) == '?') {
    parse_declaration(document, &iterator);
  }

  skip_wac(&iterator);

  if (*iterator == LEFT_ALLIGATOR) {
    Node * node = parse_node(&iterator);

    document->attribute_count = node->attribute_count;
    document->attributes = node->attributes;

    document->node_count = node->node_count;
    document->nodes = node->nodes;

    document->resource = hybridize_name(document->resource, node->identifier);
  } else {
    printf("##invalid file##\n%s", iterator);
  }
}

CXMLDocument * CXMLDocument_Load(const char * filename) {
  CXMLDocument * document = 0;

  char * buf = setup_buffer(filename);

  if (buf != 0) {
    document = CXMLDocument_Create(0);
  }

  parse_document(document);

  if (buffer != 0) {
    safe_free(buffer);

    buffer = 0;
  }

  return document;
}
