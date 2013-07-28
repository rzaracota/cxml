#include "cxml.h"

void * safe_free(void * pointer) {
  if (pointer != 0) {
    free(pointer);

    pointer = 0;
  } else {
    printf("##Attempted to free null pointer.##\n");
  }

  return pointer;
}

Node * create_node(void) {
  Node * node = (Node *)malloc(sizeof (Node));

  memset(node, 0, sizeof (Node));

  return node;
}
