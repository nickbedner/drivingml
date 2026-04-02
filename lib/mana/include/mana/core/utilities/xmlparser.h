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

global inline void xml_node_init(struct XmlNode* xml_node, char* name) {
  xml_node->name = name;
}

global inline void xml_node_delete(struct XmlNode* xml_node) {
  free(xml_node->name);
  if (xml_node->attributes != NULL) {
    const char* attributes_key;
    struct MapIter attributes_iter = map_iter();
    while ((attributes_key = map_next(xml_node->attributes, &attributes_iter)))
      free(*((char**)map_get(xml_node->attributes, attributes_key)));
    map_delete(xml_node->attributes);
    free(xml_node->attributes);
  }
  free(xml_node->data);
  if (xml_node->child_nodes != NULL) {
    const char* child_node_key;
    struct MapIter child_node_iter = map_iter();
    while ((child_node_key = map_next(xml_node->child_nodes, &child_node_iter))) {
      struct ArrayList** child_list_pointer = (struct ArrayList**)map_get(xml_node->child_nodes, child_node_key);
      array_list_delete(*child_list_pointer);
      free(*child_list_pointer);
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
  if (xml_node->child_nodes != NULL) {
    struct ArrayList** nodes = (struct ArrayList**)map_get(xml_node->child_nodes, child_name);
    if (nodes != NULL && !array_list_empty(*nodes))
      return (struct XmlNode*)array_list_get(*nodes, 0);
  }
  return NULL;
}

global inline struct XmlNode* xml_node_get_child_with_attribute(struct XmlNode* xml_node, char* child_name, char* attr, const char* value) {
  struct ArrayList* children = xml_node_get_children(xml_node, child_name);
  if (children == NULL || array_list_empty(children) || value == NULL)
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
  if (xml_node->child_nodes != NULL) {
    struct ArrayList** children = (struct ArrayList**)map_get(xml_node->child_nodes, name);
    if (children != NULL)
      return *children;
  }
  return NULL;
}

global inline void xml_node_add_attribute(struct XmlNode* xml_node, char* attr, char* value) {
  if (xml_node->attributes == NULL) {
    xml_node->attributes = (struct Map*)malloc(sizeof(struct Map));
    map_init(xml_node->attributes, sizeof(char*));
  }
  map_set(xml_node->attributes, attr, &value);
}

global inline void xml_node_add_child(struct XmlNode* xml_node, struct XmlNode* child) {
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
  } else
    array_list_add(*list, child);
}

global inline char* xml_node_get_data(struct XmlNode* xml_node) {
  return xml_node->data;
}

global inline void xml_node_set_data(struct XmlNode* xml_node, char* data) {
  xml_node->data = data;
}

global inline struct XmlNode* xml_parser_load_xml_file(const char* xml_file_path);
global inline struct XmlNode* xml_parser_load_node(char** scanner);
global inline void xml_parser_delete(struct XmlNode* xml_node);

global inline char* xml_parser_read_file(const char* filename, size_t* file_length) {
  FILE* fp = NULL;
  char* result = NULL;
  i64 size = -1;

  if (fopen_s(&fp, filename, "rb") != 0 || fp == NULL)
    return NULL;

  if (fseek(fp, 0, SEEK_END) != 0)
    goto cleanup;

  size = ftell(fp);
  if (size < 0)
    goto cleanup;

  rewind(fp);

  result = (char*)calloc(1, (size_t)(size + 1));
  if (!result)
    goto cleanup;

  if (fread(result, (size_t)size, 1, fp) != 1) {
    free(result);
    result = NULL;
  } else {
    result[size] = '\0';
    *file_length = (size_t)size;
  }

cleanup:
  fclose(fp);
  return result;
}

global inline struct XmlNode* xml_parser_load_xml_file(const char* xml_file_path) {
  size_t xml_file_length = 0;
  char* xml_file_data = xml_parser_read_file(xml_file_path, &xml_file_length);
  char* file_start = xml_file_data;

  if (strncmp(file_start, "<?", 2) == 0)
    file_start = strchr(file_start, '\n') + 1;
  char** scanner = &file_start;

  struct XmlNode* xml_node = xml_parser_load_node(scanner);

  free(xml_file_data);

  return xml_node;
}

global inline struct XmlNode* xml_parser_load_node(char** scanner) {
  // Extract line
  char* line_end = strchr(*scanner, '\n');
  if (line_end == NULL)
    line_end = strchr(*scanner, '\0');
  size_t line_length = (size_t)(line_end - *scanner);
  if (line_length == 0)
    return NULL;
  char* line = (char*)malloc(sizeof(char) * (line_length + 1));
  snprintf(line, line_length + 1, "%s", *scanner);
  *scanner += line_length + 1;

  // Trim whitespace
  char* remove_whitespace_line = line;
  while (isspace((unsigned char)*remove_whitespace_line))
    remove_whitespace_line++;

  if (strncmp(remove_whitespace_line, "</", 2) == 0) {
    free(line);
    return NULL;
  }

  // Extract tag in line
  char* tag_end = strchr(remove_whitespace_line, '>');
  size_t tag_length = (size_t)(tag_end - (remove_whitespace_line + 1));
  char* tag = (char*)malloc(sizeof(char) * (tag_length + 1));
  snprintf(tag, tag_length + 1, "%s", remove_whitespace_line + 1);

  // Split tag elements
  struct ArrayList tag_parts = {0};
  array_list_init(&tag_parts);
  char* next_token = NULL;
  char* tag_part = strtok_s(tag, " ", &next_token);
  while (tag_part != NULL) {
    array_list_add(&tag_parts, tag_part);
    tag_part = strtok_s(NULL, " ", &next_token);
  }

  // Remove slash
  struct XmlNode* xml_node = (struct XmlNode*)calloc(1, sizeof(struct XmlNode));
  char* node_name = _strdup((char*)array_list_get(&tag_parts, 0));
  char* check_slash = strchr(node_name, '/');
  // Everything after slash move left one
  if (check_slash != NULL)
    memmove_s(check_slash, strnlen(node_name, 9001), check_slash + 1, strnlen(check_slash, 9001));
  xml_node_init(xml_node, node_name);

  // Add attributes
  for (size_t tag_num = 0; tag_num < array_list_size(&tag_parts); tag_num++) {
    char* tag_text = (char*)array_list_get(&tag_parts, tag_num);
    // size_t tag_text_length = strnlen(tag_text, 9001);
    char* tag_contains_equal = strchr(tag_text, '=');
    if (tag_contains_equal != NULL) {
      size_t tag_equal_length = (size_t)(tag_contains_equal - tag_text);
      char* tag_equal_text = (char*)malloc(sizeof(char) * (tag_equal_length + 1));
      snprintf(tag_equal_text, tag_equal_length + 1, "%s", tag_text);

      // TODO: Add support for " " name tags like line 2242 of model
      char* tag_attr_start = tag_text + tag_equal_length + 2;
      char* tag_attr_end;
      // Null or empty tag value
      if (*tag_attr_start == '\0')
        tag_attr_end = tag_attr_start + 1;
      else
        tag_attr_end = strchr(tag_attr_start, '\"');
      size_t tag_value_length = (size_t)(tag_attr_end - tag_attr_start);
      char* tag_value = (char*)malloc(sizeof(char) * (tag_value_length + 1));
      snprintf(tag_value, tag_value_length + 1, "%s", tag_attr_start);

      xml_node_add_attribute(xml_node, tag_equal_text, tag_value);
      free(tag_equal_text);
    }
  }
  array_list_delete(&tag_parts);

  // Add data
  char* data_start = strchr(line, '>') + 1;
  // printf("%s\n", data_start);
  char* data_end = strchr(data_start, '<');
  if (data_end != NULL) {
    i64 data_length = data_end - data_start;
    char* data_value = (char*)malloc(sizeof(char) * ((size_t)data_length + 1));
    snprintf(data_value, (size_t)(data_length + 1), "%s", data_start);

    xml_node_set_data(xml_node, data_value);
  }

  // Check if closing (start or end)
  char* closed_end = strstr(line, "</");
  char* closed_start = strstr(line, "/>");
  if (closed_end != NULL || closed_start != NULL) {
    free(line);
    free(tag);
    return xml_node;
  }

  // Recursive all nodes
  struct XmlNode* xml_child = NULL;
  while ((xml_child = xml_parser_load_node(scanner)) != NULL)
    xml_node_add_child(xml_node, xml_child);

  // Done with current node
  free(line);
  free(tag);
  return xml_node;
}

global inline void xml_parser_delete(struct XmlNode* xml_node) {
  if (xml_node->child_nodes != NULL) {
    const char* key;
    struct MapIter iter = map_iter();
    while ((key = map_next(xml_node->child_nodes, &iter))) {
      struct ArrayList** child_list_pointer = (struct ArrayList**)map_get(xml_node->child_nodes, key);
      if (child_list_pointer != NULL) {
        for (size_t child_num = 0; child_num < array_list_size(*child_list_pointer); child_num++)
          xml_parser_delete((struct XmlNode*)array_list_get(*child_list_pointer, child_num));
      }
    }
  }
  xml_node_delete(xml_node);
  free(xml_node);
}
