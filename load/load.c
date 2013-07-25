#include "cxml.h"

#include "document/document.h"

#include "load/load.h"

static char * buffer = 0;

typedef enum _TokenType { SPACE = ' ', NEWLINE = '\n', TAB = '\t',
			  LEFT_ALLIGATOR = '<', RIGHT_ALLIGATOR = '>',
			  FORWARD_SLASH = '/', DOUBLE_QUOTE = '\"',
			  SINGLE_QUOTE = '\'', EQUALS = '='
                        } TokenType;		          

enum { true = 1, false = 0 };

static unsigned int whitespace(TokenType token) {
  if (token == SPACE || token == NEWLINE || token == TAB) {
    return true;
  }

  return false;
}

static int _iterator = 0;

static char * get_value(char * const beg) {
  char * value = 0, * iter = beg, * begin = beg;

  if (*begin != EQUALS) {
    printf("orphan attribute\n");

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

  value = (char *)malloc(sizeof (char) * (i));

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

static void add_attribute(CXMLDocument * document, char * attribute,
			  char * value) {
  CXMLDocument_AddAttribute(document, attribute, value);
}

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

    identifier = get_identifier(buffer + i + 1);

    i += _iterator + 1;

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

CXMLDocument * CXMLLoad(const char * filename) {
  CXMLDocument * document = 0;

  char * buf = setup_buffer(filename);

  if (buf != 0) {
    document = CXMLDocument_Create(filename);
  }

  parse_buffer(document);

  if (buffer != 0) {
    free(buffer);

    buffer = 0;
  }

  return document;
}
