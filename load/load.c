#include "cxml.h"

static char * buffer = 0;

typedef enum _TokenType { SPACE = ' ', NEWLINE = '\n', TAB = '\t',
			  LEFT_ALLIGATOR = '<', RIGHT_ALLIGATOR = '>',
			  FORWARD_SLASH = '/', DOUBLE_QUOTE = '\"',
			  SINGLE_QUOTE = '\'', EQUALS = '='
                        } TokenType;		          

enum { true = 1, false = 0 };

static void display_attribute(const Attribute * attribute) {
  printf("identifier: %s\n", attribute->identifier);
  printf("value: %s\n", attribute->value);
}

static void display_node(const Node * const node) {
  printf("--Node--\n");

  if (node == 0) {
    printf("##can not display null node##\n");

    return;
  }

  printf("identifier: %s\n", node->identifier);
  printf("attribute_count: %d\n", node->attribute_count);
  printf("node_count: %d\n", node->node_count);

  printf("--attributes--\n");

  for (int i = 0; i < node->attribute_count; i++) {
    display_attribute(node->attributes + i);
  }

  printf("--nodes--\n");

  for (int i = 0; i < node->node_count; i++) {
    display_node(node->nodes + i);
  }

  printf("pcdata: %s\n", node->pcdata);
}

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

static int _iterator = 0;

static void kill_comment(char * const beg) {
  int i = 0;

  while (*(beg + i) != 0) {
    if (*(beg + i) == RIGHT_ALLIGATOR &&
	*(beg + i - 1) == '-' &&
	*(beg + i - 2) == '-') {
      _iterator = i + 1;

      char * temp = (char *)malloc(sizeof (char) * (i - 2));

      strncpy(temp, beg, i - 2);

      temp[i - 2] = '\0';

      printf("comment:\n%s\n", temp);

      if (temp != 0) {
	free(temp);

	temp = 0;
      }

      break;
    }

    i++;
  }
}

static char * get_value(char * const beg) {
  char * value = 0, * iter = beg, * begin = beg;

  if (*begin != EQUALS) {
    printf("orphan attribute\n%s", beg);

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

  _iterator = i + 1 + (begin - beg);

  return value;
}

static char * get_identifier(char * const beg) {
  char * identifier = 0, * iter = beg;

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

  _iterator = i;

  identifier = (char *)malloc(sizeof (char *) * (i + 1));

  if (identifier == 0) {
    printf("##memory allocation error.##\n");

    return identifier;
  }

  strncpy(identifier, beg, i);

  identifier[i] = '\0';

  printf("identifier: |%s|\n", identifier);

  return identifier;
}

static char * get_pcdata(char * buf) {
  char * pcdata = 0, * iter = buf, * begin = buf;

  int length = 0;

  while (*iter != 0 && whitespace(*iter)) {
    while (whitespace(*iter++)) continue;

    if (comment(iter)) {
      kill_comment(iter);

      iter += _iterator;
    }
  }

  begin = iter;

  while (*iter != 0 && *iter != LEFT_ALLIGATOR) {
    iter++;
    length++;
  }

  if (length > 0) {
    pcdata = (char *)malloc(sizeof (char) * (length + 1));

    strncpy(pcdata, begin, length);

    pcdata[length] = '\0';
  }

  _iterator = iter - buf;

  return pcdata;
}

/*
 * Begin cleanup
 */

static char * clean_get_identifier(char * const beg) {
  char * identifier = 0, * iter = beg;

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

  _iterator = i;

  identifier = (char *)malloc(sizeof (char *) * (i + 1));

  if (identifier == 0) {
    printf("##memory allocation error.##\n");

    return identifier;
  }

  strncpy(identifier, beg, i);

  identifier[i] = '\0';

  printf("identifier: |%s|\n", identifier);

  return identifier;
}

/*
 * End cleanup section
 */

static Node * parse_attributes(Node * node, char * const buf);

static void get_attributes(char * const buf) {
}

static void add_attribute(CXMLDocument * document, char * attribute,
			  char * value) {
  CXMLDocument_AddAttribute(document, attribute, value);
}

static void clean_parse_declaration(CXMLDocument * document,
				    char ** iterator) {
  char * iter = *iterator, * begin = *iterator;

  if (document == 0) {
    printf("##null document##\n");

    return;
  }

  *iterator += 2;

  char * xml = get_identifier(*iterator);

  *iterator += _iterator;

  _iterator = 0;

  if (strcmp(xml, "xml") != 0) {
    printf("##invalid declaration node##\n");

    while (**iterator != RIGHT_ALLIGATOR) {
      *iterator++;
    }
  }

  Node * node = create_node();

  if (node == 0) {
    printf("##failed to allocate memory for node##\n");

    return;
  }

  node = parse_attributes(node, *iterator);

  *iterator += _iterator;

  _iterator = 0;

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

static void parse_declaration(CXMLDocument * document, char * const beg) {
  char * iter = beg, * begin = beg;

  if (document == 0) {
    printf("##null document##\n");

    return;
  }

  while (*(iter) != '\0') {
    if (whitespace(*(iter))) {
      begin++;

      iter++;

      continue;
    } else if (*(iter) == '?') {
      if (*(iter + 1) == RIGHT_ALLIGATOR) {
	_iterator = iter - beg + 2;

	return;
      } else {
	printf("Illformatted declaration.\n");

	return;
      }
    }

    char * identifier = get_identifier(iter);

    iter += _iterator;

    _iterator = 0;

    if (strcmp(identifier, "version") == 0) {
      document->declaration.version = get_value(iter);

      iter += _iterator;

      _iterator = 0;
    } else if (strcmp(identifier, "language") == 0 ||
		 strcmp(identifier, "lang") == 0) {
      document->declaration.language = get_value(iter);

      iter += _iterator;

      _iterator = 0;
    } else if (strcmp(identifier, "encoding") == 0) {
      document->declaration.encoding = get_value(iter);

      iter += _iterator;

      _iterator = 0;
    } else {
      printf("Unsupported declaration attribute: %s\n", identifier);
    }
  }

  _iterator = iter - beg;
}

static Node * node_add_attribute(Node * node,
				 Attribute * new_attribute) {
  if (node == 0) {
    printf("##cannot add attribute to null node##\n");

    return 0;
  }

  Attribute * ab =  (Attribute *)malloc(sizeof (Attribute) *
					(node->attribute_count + 1));

  if (node->attribute_count != 0) {
    memcpy(ab, node->attributes, sizeof (Attribute) * node->attribute_count);
  }

  if (false) {
    printf("node_add_attribute\n");

    printf("in:\n");

    display_attribute(new_attribute);
  }

  ab[node->attribute_count].identifier = new_attribute->identifier;
  ab[node->attribute_count].value = new_attribute->value;

  if (false) {
    printf("out:\n");

    display_attribute(ab + node->attribute_count);
  }

  node->attributes = ab;
  node->attribute_count++;

  return node;
}

static Node * node_add_node(Node * node, Node * new_node) {
  if (node == 0) {
    return node;
  }

  printf("--node_add_node--\n");

  printf("--destination node--\n");

  //display_node(node);

  printf("--new_node--\n");

  //display_node(new_node);

  printf("new_node->nodes: %p\n");

  Node * node_buffer = (Node *)malloc(sizeof (Node) * (node->node_count + 1));

  if (node->node_count > 0) {
    memcpy(node_buffer, node->nodes, sizeof (Node) * (node->node_count)); 
  }

  memcpy(node_buffer + node->node_count, new_node, sizeof (Node));

  node->node_count++;

  safe_free(node->nodes);

  node->nodes = node_buffer;

  printf("--result node--\n");

  printf("result node: %p\n", node->nodes[0].nodes);

  return node;
}

static Node * parse_attributes(Node * node, char * const buf) {
  if (node == 0) {
    printf("##null node in parse_attributes.\n##");

    return 0;
  }

  char short_string[16];

  strncpy(short_string, buf, 15);

  short_string[15] = '\0';

  printf("parse_attributes\nshort_string: |%s|\n", short_string);

  Attribute * attributes = 0;

  char * iter = buf, * begin = buf;

  if (*iter == RIGHT_ALLIGATOR ||
      *iter == LEFT_ALLIGATOR ||
      *iter == '/') {
    printf("no attributes present\n");

    //_iterator = 1;

    return node;
  } else if (whitespace(*iter)) {
    while (whitespace(*(++iter)));
  }

  char * identifier = 0;
  char * value = 0;

  while (*iter != 0) {
    while (whitespace(*(iter))) {
      iter++;
    }

    if (*iter == '/' || *iter == RIGHT_ALLIGATOR || *iter == '?') {
      break;
    }

    identifier = get_identifier(iter);

    iter += _iterator;

    _iterator = 0;

    value = get_value(iter);

    iter += _iterator;

    _iterator = 0;

    printf("attribute: ['%s'] %s\n%s", identifier, value, iter);

    if (identifier != 0) {
      Attribute attribute = { .identifier = identifier,
			      .value = value };

      printf("att: ['%s'] %s\n", attribute.identifier,
	     attribute.value);

      node = node_add_attribute(node,
				&((Attribute) { identifier, value }));

      identifier = 0;

      value = 0;
    }
  }

  _iterator = iter - buf;

  //printf("parse_attributes->display_node;\n");

  //display_node(node);

  return node;
}

static void skip_comment(char ** iterator) {
  char * beg = *iterator;

  int i = 0;

  while (*(beg + i) != 0) {
    if (*(beg + i) == RIGHT_ALLIGATOR &&
	*(beg + i - 1) == '-' &&
	*(beg + i - 2) == '-') {
      _iterator = i + 1;

      char * temp = (char *)malloc(sizeof (char) * (i - 2));

      strncpy(temp, beg, i - 2);

      temp[i - 2] = '\0';

      printf("comment:\n%s\n", temp);

      if (temp != 0) {
	free(temp);

	temp = 0;
      }

      break;
    }

    i++;
  }

  *iterator += i + 1;
}

static void skip_wac(char ** iterator) {
  while (**iterator != 0) {
    if (comment(*iterator)) {
      skip_comment(&(*iterator));
    } else if (!whitespace(**iterator)) {
      break;
    }

    (*iterator)++;
  }
}

static char * clean_get_pcdata(char ** iterator) {
  printf("==clean_get_pcdata==\n%s\n", *iterator);

  char * pcdata = 0, * iter = *iterator, * begin = *iterator;

  int length = 0;

  begin = iter;

  while (*iter != 0 && *iter != LEFT_ALLIGATOR) {
    iter++;
    length++;
  }

  if (length > 0) {
    pcdata = (char *)malloc(sizeof (char) * (length + 1));

    strncpy(pcdata, begin, length);

    pcdata[length] = '\0';
  }

  (*iterator) += iter - begin;

  return pcdata;
}

static Node * parse_opening_element(char ** iterator) {
  printf("parse_opening_element\n%s", *iterator);

  if (**iterator != LEFT_ALLIGATOR) {
    return 0;
  }

  Node * node = create_node();

  (*iterator)++;

  char * shield = *iterator;

  char * identifier = get_identifier(shield);

  (*iterator) += _iterator;

  _iterator = 0;

  if (identifier == 0) {
    safe_free(node);

    return 0;
  }

  node->identifier = (char *)malloc(sizeof (char) * (strlen(identifier) + 1));

  strcpy(node->identifier, identifier);

  if (**iterator == ' ') {
    shield = *iterator;

    node = parse_attributes(node, shield);

    *iterator += _iterator;

    _iterator = 0;
  }

  if (**iterator == '/') {
    (*iterator)++;
  }

  (*iterator)++;

  return node;
}

static Node * parse_closing_element(Node * node, char ** iterator) {
  printf("parse_closing_element\n%s", *iterator);

  if (node == 0) {
    return 0;
  }

  (*iterator)++;

  if (**iterator != '/') {
    return node;
  }

  (*iterator)++;

  char * identifier = get_identifier(*iterator);

  *iterator += _iterator;

  _iterator = 0;

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

static Node * clean_parse_node(char ** iterator) {
  skip_wac(&(*iterator));

  printf("===================clean_parse_node==================\n");

  Node * node = 0;

  node = parse_opening_element(&(*iterator));

  if (node == 0) {
    printf("##invalid node##\n");
  
    return 0;
  }

  if (*(*iterator - 1) != RIGHT_ALLIGATOR) {
    printf("##invalid opening element##\n");

    return 0;
  } else if (*(*iterator - 2) == '/') {
    return node;
  }

  while (*(*iterator) != 0) {
    skip_wac(&(*iterator));

    if (*(*iterator) != LEFT_ALLIGATOR) {
      node->pcdata = clean_get_pcdata(&(*iterator));
    } else if (*(*iterator + 1) != '/') {
      Node * new_node = clean_parse_node(&(*iterator));

      if (new_node != 0) {
	printf("new_node\n");

	display_node(new_node);

	node = node_add_node(node, new_node);

	safe_free(new_node);
      }
    } else {
      parse_closing_element(node, iterator);

      break;
    }
  }

  return node;
}

static char * _pcdata = 0;

static int is_pcdata = 0;

static Node * parse_node(char * const buf) {
  char * iter = buf;

  Node * node = 0;

  char * identifier = 0;

  Attribute * attributes = 0;

  if (whitespace(*iter)) {
    printf("##invalid space in node identifier.##\n");

    return 0;
  } else if (*iter == LEFT_ALLIGATOR) {
    if (*(++iter) == '/') {
      printf("close node\n%s\n", iter);

      _iterator = iter - buf;

      printf("node: %p\n", node);

      return node;
    } else {
      identifier = get_identifier(iter);

      iter += _iterator;

      _iterator = 0;

      node = (Node *)malloc(sizeof (Node));

      node->identifier = identifier;

      node->attribute_count = 0;
      node->attributes = 0;

      node->node_count = 0;
      node->nodes = 0;

      printf("node->identifier: %s\n", node->identifier);
    }      
  } else {
    printf("##parse_node: this should not happen. %c##\n",
	   *iter);

    return 0;
  }

  while (whitespace(*iter)) iter++;

  node = parse_attributes(node, iter);

  iter += _iterator;

  _iterator = 0;

  printf("after parse_attributes:\n%s", iter);

  if (*iter == RIGHT_ALLIGATOR) {
    iter++;

    printf("right_alligator - finish element\n");
    printf("%s\n", iter);

    while (iter != 0) {
      if (whitespace(*(iter))) {
	iter++;

	continue;
      } else if (comment(iter)) {
	kill_comment(iter);

	iter += _iterator;
      } else if (*iter == LEFT_ALLIGATOR) {
	printf("left_alligator - new node\n");
	printf("%s\n", iter);

	Node * n = parse_node(iter);

	if (node != 0 && false) {
	  printf("-- display_node(node)--\n");

	  display_node(node);
	}

	if (n != 0 && false) {
	  printf("-- display_node(n)--\n");

	  display_node(n);
	}
	
	iter += _iterator;

	_iterator = 0;

	printf("left alligator:\n%s\n", iter);

	if (n != 0) {
	  if (is_pcdata == 1) {
	    node->pcdata = (char *)n;

	    is_pcdata = 0;
	  } else {
	    node = node_add_node(node, n);
	  }

	  //printf("node_add_node no ato\n");

	  //display_node(node);

	  safe_free(n);
	} else if (*iter == '/') {
	  printf("closing node.\n%s", iter);

	  char * id = 0;

	  id = get_identifier(iter + 1);
	  
	  iter += _iterator + 1;
	  
	  if (id != 0 && strcmp(node->identifier, id) == 0) {
	    printf("tags actually matched.\nclosing %s..\n", id);

	    safe_free(id);

	    while (*iter != RIGHT_ALLIGATOR && *iter != 0) {
	      iter++;
	    }

	    if (*iter != RIGHT_ALLIGATOR) {
	      printf("##broken closing tag.##\n");

	      return node;
	    }

	    iter++;

	    printf("closing node node:%s\n", iter);

	    //display_node(node);

	    printf("closing node node displayed successfully.\n");

	    break;
	  } else {
	    printf("mismatched element tags.\n");
	  }

	} else {
	  printf("##unexpected null node in parse_node##\nctr%c", *iter);

	  return 0;
	}
      } else {
	printf("it should be pcdata\n");
	printf("character: %c\n", *iter);

        char short_string[16];

	strncpy(short_string, iter, 15);

	short_string[15] = '\0';

	printf("pcdata\nshort_string: |%s|\n", short_string);

	char * pcdata = get_pcdata(iter);

	printf("result pcdata: %s\n", pcdata);

	_pcdata = pcdata;

	is_pcdata = 1;

	iter += _iterator;

	//_iterator = 0;

	//_iterator = iter - buf;

	if (node->nodes != 0 && node->pcdata != 0) {
	  printf("##unexpected nodes or pcdata found##\n");

	  //display_node(node);
	} else {
	  node->pcdata = pcdata;
	}

	printf("comfirming node closure.\n");

	if (*iter != LEFT_ALLIGATOR &&
	    *(iter + 1) != '/') {
	  printf("##improper data after pcdata##\n%s", iter);
	} else {
	  char * id = get_identifier(iter + 2);

	  iter += _iterator + 3;

	  if (id == 0) {
	    printf("##invalid closing tag##\n");

	    return node;
	  }

	  if (strcmp(node->identifier, id) != 0) {
	    printf("mismatched closing element.\n");
	  }

	  printf("leaving parse node with\n%s\n", iter);

	  break;
	}
      }
    }
  } else if (*iter == '/' && *(iter + 1) == RIGHT_ALLIGATOR) {
    printf("close singleton node\n%s", iter + 2);

    iter += 2;
  } else {
    printf("##trailing / in attributes.##\n");
  }

  _iterator = iter - buf;

  printf("pre-exit node:\n");

  display_node(node);

  printf("returning..\n");

  return node;
}

static Node * parse_nodes(char * const beg) {
  char * iter = beg, * begin = beg;

  Node * node = 0;

  for (; *iter != 0; iter++) {
    if (whitespace(*iter)) continue;

    if (comment(iter)) {
      kill_comment(iter);

      iter += _iterator + 4;

      _iterator = 0;
    }

    if (*iter == LEFT_ALLIGATOR) {
      node = parse_node(iter);

      iter += _iterator;

      _iterator = 0;
    }
  }  

  return node;
}

static void parse_buffer(CXMLDocument * document) {
  if (buffer == 0) {
    printf("Invalid buffer.\n");

    return;
  }

  char * identifier = 0;

  int i = 0;

  while (*(buffer + i) != 0) {
    for (; buffer[i] != LEFT_ALLIGATOR && whitespace(buffer[i]); i++);

    if (buffer[i] != LEFT_ALLIGATOR) {
      printf("invalid initial character.\n");
      printf("buffer[%d]: %c\n", i, buffer[i]);

      return;
    }

    if (*(buffer + i + 1) == '!' &&
	*(buffer + i + 2) == '-' &&
	*(buffer + i + 3) == '-') {
      kill_comment(buffer + i + 4);

      i += _iterator + 4;

      _iterator = 0;

      continue;
    }    

    char * begin = buffer + i;

    identifier = get_identifier(buffer + i + 1);

    i += _iterator + 1;

    _iterator = 0;

    if (strcmp(identifier, "?xml") == 0) {
      printf("xml declaration initial found; attempting to parse.\n");

      parse_declaration(document, buffer + i);

      i += _iterator + 1;

      if (identifier != 0) {
	free(identifier);

	identifier = 0;
      }
    } else {
      char * hybrid_name = 0;

      if (document->resource != 0) {
	hybrid_name = (char *)malloc(sizeof (char) *
				     (strlen(document->resource) +
				      (strlen(identifier) + 1 + 1)));
	strncpy(hybrid_name, document->resource, strlen(document->resource));
	hybrid_name[strlen(document->resource)] = '#';

	strcpy(hybrid_name + strlen(document->resource) + 1, identifier);

	document->resource = hybrid_name;
      } else {
	document->resource = identifier;
      }

      if (identifier != 0) {
	free(identifier);

	identifier = 0;
      }

      Node * document_node = parse_nodes(begin);

      printf("=============================document_node===================\n");

      display_node(document_node);

      document->resource = document_node->identifier;
      document->node_count = document_node->node_count;
      document->nodes = document_node->nodes;

      document->attribute_count = document_node->attribute_count;
      document->attributes = document_node->attributes;

      return;

      int j = i;

      char * attribute = 0;
      char * value = 0;

      for (; buffer[j] != RIGHT_ALLIGATOR;) {
	while(whitespace(buffer[j])) j++;

	attribute = get_identifier(buffer + j);

	j += _iterator;

	printf("%s$%c\n", attribute, buffer[j]);

	value = get_value(buffer + j);

	j += _iterator;

	printf("%s$%c\n", value, buffer[j]);

	if (attribute != 0) {
	  if (value == 0) {
	    printf("##Invalid attribute specification.##\n");

	    return;
	  }
	  
	  CXMLDocument_AddAttribute(document, attribute, value);

	  //attribute = safe_free(attribute);

	  //value = safe_free(value);

	  document->attribute_count++;

	  attribute = 0;
	  value = 0;
	}
      }

      if (buffer[j] == RIGHT_ALLIGATOR) {
	if (buffer[j - 1] == '/') {
	  printf("##null document##\n");

	  return;
	} else {
	  printf("\033[0;32mdocument node parsing complete.\033[0m\n");

	  Node * node = parse_nodes(buffer + j + 1);

	  if (node != 0) {
	    document->node_count = node->node_count;

	    document->nodes = node->nodes;
	  } else {
	    printf("##document nodes are null.##\n");
	  }

	  return;
	}
      }
    }
  }
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

CXMLDocument * CXMLDocument_Load(const char * filename) {
  CXMLDocument * document = 0;

  char * buf = setup_buffer(filename);

  if (buf != 0) {
    document = CXMLDocument_Create(filename);
  }

  //parse_buffer(document);

  printf("====================clean_parse_node==================\n");

  //CXMLDocument_Destroy(document);

  char * iterator = buffer;

  skip_wac(&iterator);

  if (*iterator == LEFT_ALLIGATOR && *(iterator + 1) == '?') {
    clean_parse_declaration(document, &iterator);
  }

  skip_wac(&iterator);

  if (*iterator == LEFT_ALLIGATOR) {
    Node * node = clean_parse_node(&iterator);

    document->attribute_count = node->attribute_count;
    document->attributes = node->attributes;

    document->node_count = node->node_count;
    document->nodes = node->nodes;

    document->resource = hybridize_name(document->resource, node->identifier);
  } else {
    printf("##invalid file##\n%s", iterator);
  }

  if (buffer != 0) {
    safe_free(buffer);

    buffer = 0;
  }

  return document;
}
