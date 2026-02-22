// static int jsoneq(const char* json, jsmntok_t* tok, const char* s) {
//   if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start && strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
//     return 0;
//   }
//   return -1;
// }
//
//  int file_length;
//  const char* model_file = read_file("./assets/models/Cube/glTF/Cube.gltf", &file_length);
//
//  jsmn_parser p;
//  jsmntok_t t[512] = {0}; /* We expect no more than 128 JSON tokens */
//
//  jsmn_init(&p);
//  int r = jsmn_parse(&p, model_file, strlen(model_file), t, 512);  // "s" is the char array holding the json content
//
//  struct GLTF cube = {0};
//  gltf_parse(&cube, model_file);

///* Loop over all keys of the root object */
///*for (int i = 1; i < r; i++) {
//  if (jsoneq(model_file, &t[i], "user") == 0) {
//    /* We may use strndup() to fetch string value */
//    printf("- User: %.*s\n", t[i + 1].end - t[i + 1].start, model_file + t[i + 1].start);
//    i++;
//  } else if (jsoneq(model_file, &t[i], "admin") == 0) {
//    /* We may additionally check if the value is either "true" or "false" */
//    printf("- Admin: %.*s\n", t[i + 1].end - t[i + 1].start, model_file + t[i + 1].start);
//    i++;
//  } else if (jsoneq(model_file, &t[i], "uid") == 0) {
//    /* We may want to do strtol() here to get numeric value */
//    printf("- UID: %.*s\n", t[i + 1].end - t[i + 1].start, model_file + t[i + 1].start);
//    i++;
//  } else if (jsoneq(model_file, &t[i], "groups") == 0) {
//    int j;
//    printf("- Groups:\n");
//    if (t[i + 1].type != JSMN_ARRAY) {
//      continue; /* We expect groups to be an array of strings */
//    }
//    for (j = 0; j < t[i + 1].size; j++) {
//      jsmntok_t* g = &t[i + j + 2];
//      printf("  * %.*s\n", g->end - g->start, model_file + g->start);
//    }
//    i += t[i + 1].size + 1;
//  } else {
//    printf("Unexpected key: %.*s\n", t[i].end - t[i].start, model_file + t[i].start);
//  }
//}*/
//