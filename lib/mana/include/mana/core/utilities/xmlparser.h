#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define WINDOWS
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <ctype.h>
#include <mana/core/storage/storage.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mana/core/corecommon.h"

struct XmlNode {
  char* name;
  struct Map* attributes;
  char* data;
  struct Map* child_nodes;  // Children are stored in an arraylist due to repeated tags
};

global inline void xml_node_init(struct XmlNode* xml_node, char* name);
global inline void xml_node_delete(struct XmlNode* xml_node);
global inline char* xml_node_get_attribute(struct XmlNode* xml_node, char* attr);
global inline struct XmlNode* xml_node_get_child(struct XmlNode* xml_node, char* child_name);
global inline struct XmlNode* xml_node_get_child_with_attribute(struct XmlNode* xml_node, char* child_name, char* attr, const char* value);
global inline struct ArrayList* xml_node_get_children(struct XmlNode* xml_node, char* name);
global inline void xml_node_add_attribute(struct XmlNode* xml_node, char* attr, char* value);
global inline void xml_node_add_child(struct XmlNode* xml_node, struct XmlNode* child);
global inline char* xml_node_get_data(struct XmlNode* xml_node);
global inline void xml_node_set_data(struct XmlNode* xml_node, char* data);

global inline struct XmlNode* xml_parser_load_xml_file(const char* xml_file_path);
global inline struct XmlNode* xml_parser_load_node(char** scanner);
global inline void xml_parser_delete(struct XmlNode* xml_node);

// =========================
// XmlNode helpers
// =========================

global inline void xml_node_init(struct XmlNode* xml_node, char* name) {
  if (xml_node == NULL)
    return;

  xml_node->name = name;
  xml_node->attributes = NULL;
  xml_node->data = NULL;
  xml_node->child_nodes = NULL;
}

global inline void xml_node_delete(struct XmlNode* xml_node) {
  if (xml_node == NULL)
    return;

  free(xml_node->name);

  if (xml_node->attributes != NULL) {
    const char* attributes_key;
    struct MapIter attributes_iter = map_iter();
    while ((attributes_key = map_next(xml_node->attributes, &attributes_iter)) != NULL) {
      char** value = (char**)map_get(xml_node->attributes, attributes_key);
      if (value != NULL)
        free(*value);
    }

    map_delete(xml_node->attributes);
    free(xml_node->attributes);
  }

  free(xml_node->data);

  if (xml_node->child_nodes != NULL) {
    const char* child_node_key;
    struct MapIter child_node_iter = map_iter();
    while ((child_node_key = map_next(xml_node->child_nodes, &child_node_iter)) != NULL) {
      struct ArrayList** child_list_pointer = (struct ArrayList**)map_get(xml_node->child_nodes, child_node_key);
      if (child_list_pointer != NULL) {
        array_list_delete(*child_list_pointer);
        free(*child_list_pointer);
      }
    }

    map_delete(xml_node->child_nodes);
    free(xml_node->child_nodes);
  }
}

global inline char* xml_node_get_attribute(struct XmlNode* xml_node, char* attr) {
  if (xml_node == NULL || xml_node->attributes == NULL || attr == NULL)
    return NULL;

  char** value = (char**)map_get(xml_node->attributes, attr);
  if (value == NULL)
    return NULL;

  return *value;
}

global inline struct XmlNode* xml_node_get_child(struct XmlNode* xml_node, char* child_name) {
  if (xml_node == NULL || child_name == NULL || xml_node->child_nodes == NULL)
    return NULL;

  struct ArrayList** nodes = (struct ArrayList**)map_get(xml_node->child_nodes, child_name);
  if (nodes != NULL && !array_list_empty(*nodes))
    return (struct XmlNode*)array_list_get(*nodes, 0);

  return NULL;
}

global inline struct XmlNode* xml_node_get_child_with_attribute(struct XmlNode* xml_node, char* child_name, char* attr, const char* value) {
  struct ArrayList* children = xml_node_get_children(xml_node, child_name);
  if (children == NULL || array_list_empty(children) || attr == NULL || value == NULL)
    return NULL;

  for (size_t child_num = 0; child_num < array_list_size(children); child_num++) {
    struct XmlNode* child = (struct XmlNode*)array_list_get(children, child_num);
    char* val = xml_node_get_attribute(child, attr);
    if (val != NULL && strcmp(value, val) == 0)
      return child;
  }

  return NULL;
}

global inline struct ArrayList* xml_node_get_children(struct XmlNode* xml_node, char* name) {
  if (xml_node == NULL || name == NULL || xml_node->child_nodes == NULL)
    return NULL;

  struct ArrayList** children = (struct ArrayList**)map_get(xml_node->child_nodes, name);
  if (children != NULL)
    return *children;

  return NULL;
}

global inline void xml_node_add_attribute(struct XmlNode* xml_node, char* attr, char* value) {
  if (xml_node == NULL || attr == NULL || value == NULL)
    return;

  if (xml_node->attributes == NULL) {
    xml_node->attributes = (struct Map*)malloc(sizeof(struct Map));
    map_init(xml_node->attributes, sizeof(char*));
  }

  map_set(xml_node->attributes, attr, &value);
}

global inline void xml_node_add_child(struct XmlNode* xml_node, struct XmlNode* child) {
  if (xml_node == NULL || child == NULL || child->name == NULL)
    return;

  if (xml_node->child_nodes == NULL) {
    xml_node->child_nodes = (struct Map*)malloc(sizeof(struct Map));
    map_init(xml_node->child_nodes, sizeof(struct ArrayList*));
  }

  struct ArrayList** list = (struct ArrayList**)map_get(xml_node->child_nodes, child->name);
  if (list == NULL) {
    struct ArrayList* new_list = (struct ArrayList*)malloc(sizeof(struct ArrayList));
    array_list_init(new_list);
    map_set(xml_node->child_nodes, child->name, &new_list);
    array_list_add(new_list, child);
  } else {
    array_list_add(*list, child);
  }
}

global inline char* xml_node_get_data(struct XmlNode* xml_node) {
  if (xml_node == NULL)
    return NULL;

  return xml_node->data;
}

global inline void xml_node_set_data(struct XmlNode* xml_node, char* data) {
  if (xml_node == NULL)
    return;

  free(xml_node->data);
  xml_node->data = data;
}

// =========================
// Parser helpers
// =========================

static inline char* xml_parser_strdup_range(const char* start, size_t length) {
  char* out = (char*)malloc(length + 1);
  if (out == NULL)
    return NULL;

  if (length > 0)
    memcpy(out, start, length);

  out[length] = '\0';
  return out;
}

static inline char* xml_parser_strdup_cstr(const char* text) {
  if (text == NULL)
    return NULL;

  return xml_parser_strdup_range(text, strlen(text));
}

static inline void xml_parser_skip_whitespace(char** scanner) {
  if (scanner == NULL || *scanner == NULL)
    return;

  while (**scanner != '\0' && isspace((unsigned char)**scanner))
    (*scanner)++;
}

static inline void xml_parser_skip_utf8_bom(char** scanner) {
  if (scanner == NULL || *scanner == NULL)
    return;

  unsigned char* p = (unsigned char*)(*scanner);
  if (p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF)
    *scanner += 3;
}

static inline b8 xml_parser_starts_with(const char* text, const char* prefix) {
  if (text == NULL || prefix == NULL)
    return FALSE;

  while (*prefix != '\0') {
    if (*text != *prefix)
      return FALSE;
    text++;
    prefix++;
  }

  return TRUE;
}

static inline void xml_parser_skip_processing_instruction(char** scanner) {
  if (scanner == NULL || *scanner == NULL)
    return;

  if (!xml_parser_starts_with(*scanner, "<?"))
    return;

  *scanner += 2;
  while (**scanner != '\0') {
    if ((*scanner)[0] == '?' && (*scanner)[1] == '>') {
      *scanner += 2;
      return;
    }
    (*scanner)++;
  }
}

static inline void xml_parser_skip_comment(char** scanner) {
  if (scanner == NULL || *scanner == NULL)
    return;

  if (!xml_parser_starts_with(*scanner, "<!--"))
    return;

  *scanner += 4;
  while (**scanner != '\0') {
    if ((*scanner)[0] == '-' && (*scanner)[1] == '-' && (*scanner)[2] == '>') {
      *scanner += 3;
      return;
    }
    (*scanner)++;
  }
}

static inline void xml_parser_skip_declaration(char** scanner) {
  if (scanner == NULL || *scanner == NULL)
    return;

  if (!xml_parser_starts_with(*scanner, "<!"))
    return;

  if (xml_parser_starts_with(*scanner, "<!--")) {
    xml_parser_skip_comment(scanner);
    return;
  }

  // Skip things like <!DOCTYPE ...>, <!ENTITY ...>, etc.
  *scanner += 2;

  char quote = '\0';
  i32 bracket_depth = 0;

  while (**scanner != '\0') {
    char c = **scanner;

    if (quote != '\0') {
      if (c == quote)
        quote = '\0';
      (*scanner)++;
      continue;
    }

    if (c == '"' || c == '\'') {
      quote = c;
      (*scanner)++;
      continue;
    }

    if (c == '[') {
      bracket_depth++;
      (*scanner)++;
      continue;
    }

    if (c == ']') {
      if (bracket_depth > 0)
        bracket_depth--;
      (*scanner)++;
      continue;
    }

    if (c == '>' && bracket_depth == 0) {
      (*scanner)++;
      return;
    }

    (*scanner)++;
  }
}

static inline char* xml_parser_decode_entities(const char* text, size_t length) {
  if (text == NULL)
    return NULL;

  char* out = (char*)malloc(length + 1);
  if (out == NULL)
    return NULL;

  size_t src = 0;
  size_t dst = 0;

  while (src < length) {
    if (text[src] == '&') {
      size_t remaining = length - src;

      if (remaining >= 5 && strncmp(text + src, "&amp;", 5) == 0) {
        out[dst++] = '&';
        src += 5;
        continue;
      }
      if (remaining >= 4 && strncmp(text + src, "&lt;", 4) == 0) {
        out[dst++] = '<';
        src += 4;
        continue;
      }
      if (remaining >= 4 && strncmp(text + src, "&gt;", 4) == 0) {
        out[dst++] = '>';
        src += 4;
        continue;
      }
      if (remaining >= 6 && strncmp(text + src, "&quot;", 6) == 0) {
        out[dst++] = '"';
        src += 6;
        continue;
      }
      if (remaining >= 6 && strncmp(text + src, "&apos;", 6) == 0) {
        out[dst++] = '\'';
        src += 6;
        continue;
      }
    }

    out[dst++] = text[src++];
  }

  out[dst] = '\0';
  return out;
}

static inline void xml_parser_append_owned_text(char** dst, char* text_to_append) {
  if (dst == NULL || text_to_append == NULL)
    return;

  if (*dst == NULL) {
    *dst = text_to_append;
    return;
  }

  size_t old_len = strlen(*dst);
  size_t add_len = strlen(text_to_append);

  char* combined = (char*)realloc(*dst, old_len + add_len + 1);
  if (combined == NULL) {
    free(text_to_append);
    return;
  }

  memcpy(combined + old_len, text_to_append, add_len + 1);
  *dst = combined;
  free(text_to_append);
}

static inline char* xml_parser_trim_in_place(char* text) {
  if (text == NULL)
    return NULL;

  char* start = text;
  while (*start != '\0' && isspace((unsigned char)*start))
    start++;

  char* end = text + strlen(text);
  while (end > start && isspace((unsigned char)end[-1]))
    end--;

  size_t new_len = (size_t)(end - start);
  if (new_len == 0) {
    free(text);
    return NULL;
  }

  if (start != text)
    memmove(text, start, new_len);

  text[new_len] = '\0';
  return text;
}

static inline b8 xml_parser_is_name_char(char c) {
  /* Loose XML name parsing; good enough for COLLADA/XML identifiers. */
  return (b8)(isalnum((unsigned char)c) || c == '_' || c == '-' || c == ':' || c == '.');
}

static inline char* xml_parser_parse_name(char** scanner) {
  if (scanner == NULL || *scanner == NULL)
    return NULL;

  const char* start = *scanner;
  while (**scanner != '\0' && xml_parser_is_name_char(**scanner))
    (*scanner)++;

  if (*scanner == start)
    return NULL;

  return xml_parser_strdup_range(start, (size_t)(*scanner - start));
}

static inline char* xml_parser_parse_attribute_value(char** scanner) {
  if (scanner == NULL || *scanner == NULL)
    return NULL;

  xml_parser_skip_whitespace(scanner);

  if (**scanner == '"' || **scanner == '\'') {
    char quote = **scanner;
    (*scanner)++;

    const char* start = *scanner;
    while (**scanner != '\0' && **scanner != quote)
      (*scanner)++;

    size_t length = (size_t)(*scanner - start);
    char* decoded = xml_parser_decode_entities(start, length);

    if (**scanner == quote)
      (*scanner)++;

    return decoded;
  }

  // Unquoted fallback
  const char* start = *scanner;
  while (**scanner != '\0' && !isspace((unsigned char)**scanner) && **scanner != '>' && !(**scanner == '/' && (*scanner)[1] == '>'))
    (*scanner)++;

  return xml_parser_decode_entities(start, (size_t)(*scanner - start));
}

static inline void xml_parser_consume_closing_tag(char** scanner, const char* expected_name) {
  (void)expected_name;

  if (scanner == NULL || *scanner == NULL || !xml_parser_starts_with(*scanner, "</"))
    return;

  *scanner += 2;
  xml_parser_skip_whitespace(scanner);

  char* closing_name = xml_parser_parse_name(scanner);
  free(closing_name);

  xml_parser_skip_whitespace(scanner);
  if (**scanner == '>')
    (*scanner)++;
}

static inline void xml_parser_skip_misc(char** scanner) {
  if (scanner == NULL || *scanner == NULL)
    return;

  for (;;) {
    xml_parser_skip_whitespace(scanner);

    if (xml_parser_starts_with(*scanner, "<?")) {
      xml_parser_skip_processing_instruction(scanner);
      continue;
    }

    if (xml_parser_starts_with(*scanner, "<!--")) {
      xml_parser_skip_comment(scanner);
      continue;
    }

    if (xml_parser_starts_with(*scanner, "<!")) {
      xml_parser_skip_declaration(scanner);
      continue;
    }

    break;
  }
}

// =========================
// File loading
// =========================

global inline char* xml_parser_read_file(const char* filename, size_t* file_length) {
  FILE* fp = NULL;
  char* result = NULL;
  i64 size = -1;

  if (file_length != NULL)
    *file_length = 0;

#ifdef WINDOWS
  if (fopen_s(&fp, filename, "rb") != 0 || fp == NULL)
    return NULL;
#else
  fp = fopen(filename, "rb");
  if (fp == NULL)
    return NULL;
#endif

  if (fseek(fp, 0, SEEK_END) != 0)
    goto cleanup;

  size = ftell(fp);
  if (size < 0)
    goto cleanup;

  rewind(fp);

  result = (char*)calloc(1, (size_t)size + 1);
  if (result == NULL)
    goto cleanup;

  if (fread(result, 1, (size_t)size, fp) != (size_t)size) {
    free(result);
    result = NULL;
    goto cleanup;
  }

  result[size] = '\0';
  if (file_length != NULL)
    *file_length = (size_t)size;

cleanup:
  fclose(fp);
  return result;
}

// =========================
// Recursive parser
// =========================

global inline struct XmlNode* xml_parser_load_node(char** scanner) {
  if (scanner == NULL || *scanner == NULL)
    return NULL;

  xml_parser_skip_misc(scanner);

  if (**scanner == '\0')
    return NULL;

  if (xml_parser_starts_with(*scanner, "</"))
    return NULL;

  if (**scanner != '<')
    return NULL;

  /* Opening tag */
  (*scanner)++; /* skip '<' */

  char* node_name = xml_parser_parse_name(scanner);
  if (node_name == NULL)
    return NULL;

  struct XmlNode* xml_node = (struct XmlNode*)calloc(1, sizeof(struct XmlNode));
  xml_node_init(xml_node, node_name);

  // Attributes / end of start tag
  for (;;) {
    xml_parser_skip_whitespace(scanner);

    if ((*scanner)[0] == '/' && (*scanner)[1] == '>') {
      *scanner += 2;
      return xml_node;
    }

    if (**scanner == '>') {
      (*scanner)++;
      break;
    }

    if (**scanner == '\0')
      return xml_node;

    char* attr_name = xml_parser_parse_name(scanner);
    if (attr_name == NULL) {
      // Skip one character to avoid infinite loops on malformed input
      (*scanner)++;
      continue;
    }

    xml_parser_skip_whitespace(scanner);

    char* attr_value = NULL;
    if (**scanner == '=') {
      (*scanner)++;
      attr_value = xml_parser_parse_attribute_value(scanner);
    } else {
      attr_value = xml_parser_strdup_cstr("");
    }

    if (attr_value == NULL)
      attr_value = xml_parser_strdup_cstr("");

    xml_node_add_attribute(xml_node, attr_name, attr_value);
    free(attr_name);
  }

  /* Content */
  char* accumulated_text = NULL;

  for (;;) {
    if (**scanner == '\0')
      break;

    if (xml_parser_starts_with(*scanner, "</")) {
      xml_parser_consume_closing_tag(scanner, xml_node->name);
      break;
    }

    if (xml_parser_starts_with(*scanner, "<!--")) {
      xml_parser_skip_comment(scanner);
      continue;
    }

    if (xml_parser_starts_with(*scanner, "<?")) {
      xml_parser_skip_processing_instruction(scanner);
      continue;
    }

    if (xml_parser_starts_with(*scanner, "<![CDATA[")) {
      *scanner += 9;
      const char* cdata_start = *scanner;

      while (**scanner != '\0' && !((*scanner)[0] == ']' && (*scanner)[1] == ']' && (*scanner)[2] == '>'))
        (*scanner)++;

      {
        size_t cdata_length = (size_t)(*scanner - cdata_start);
        char* cdata_text = xml_parser_strdup_range(cdata_start, cdata_length);
        xml_parser_append_owned_text(&accumulated_text, cdata_text);
      }

      if ((*scanner)[0] == ']' && (*scanner)[1] == ']' && (*scanner)[2] == '>')
        *scanner += 3;

      continue;
    }

    if (xml_parser_starts_with(*scanner, "<!")) {
      xml_parser_skip_declaration(scanner);
      continue;
    }

    if (**scanner == '<') {
      struct XmlNode* child = xml_parser_load_node(scanner);
      if (child != NULL) {
        xml_node_add_child(xml_node, child);
        continue;
      }

      // Prevent infinite loop on malformed input
      if (**scanner == '<')
        (*scanner)++;
      continue;
    }

    // Text node
    {
      const char* text_start = *scanner;
      while (**scanner != '\0' && **scanner != '<')
        (*scanner)++;

      {
        size_t text_length = (size_t)(*scanner - text_start);
        char* decoded_text = xml_parser_decode_entities(text_start, text_length);
        xml_parser_append_owned_text(&accumulated_text, decoded_text);
      }
    }
  }

  accumulated_text = xml_parser_trim_in_place(accumulated_text);
  if (accumulated_text != NULL)
    xml_node_set_data(xml_node, accumulated_text);

  return xml_node;
}

global inline struct XmlNode* xml_parser_load_xml_file(const char* xml_file_path) {
  size_t xml_file_length = 0;
  char* xml_file_data = xml_parser_read_file(xml_file_path, &xml_file_length);
  if (xml_file_data == NULL)
    return NULL;

  char* scanner = xml_file_data;
  xml_parser_skip_utf8_bom(&scanner);
  xml_parser_skip_misc(&scanner);

  struct XmlNode* xml_node = xml_parser_load_node(&scanner);

  free(xml_file_data);
  return xml_node;
}

global inline void xml_parser_delete(struct XmlNode* xml_node) {
  if (xml_node == NULL)
    return;

  if (xml_node->child_nodes != NULL) {
    const char* key;
    struct MapIter iter = map_iter();

    while ((key = map_next(xml_node->child_nodes, &iter)) != NULL) {
      struct ArrayList** child_list_pointer = (struct ArrayList**)map_get(xml_node->child_nodes, key);
      if (child_list_pointer != NULL) {
        for (size_t child_num = 0; child_num < array_list_size(*child_list_pointer); child_num++) {
          struct XmlNode* child = (struct XmlNode*)array_list_get(*child_list_pointer, child_num);
          xml_parser_delete(child);
        }
      }
    }
  }

  xml_node_delete(xml_node);
  free(xml_node);
}
