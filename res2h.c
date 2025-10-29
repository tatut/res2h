/* Resource to header file */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "lzav/lzav.h"

int main(int argc, char **argv) {
  if(argc < 2) {
    printf("Usage: res2h output.h <res1... resN>");
    return 1;
  }
  char *header_file = argv[1];
  printf("Output to: %s\n", header_file);
  FILE *out = fopen(header_file, "w");
  if(!out) {
    printf("Could not open %s for writing.\n", header_file);
    return 1;
  }
  fprintf(out,
          "#include <stdint.h>\n"
          "#include <string.h>\n"
          "#include <stdbool.h>\n"
          "#include \"lzav/lzav.h\"\n"
          "#ifndef RES2H_ALLOC\n"
          "#define RES2H_ALLOC(size) malloc((size))\n"
          "#endif\n");

  for(int i=2; i < argc; i++) {
    FILE *in = fopen(argv[i], "rb");
    if(!in) {
      printf("Could not open %s for reading.\n", argv[i]);
      return 1;
    }
    struct stat st;
    fstat(fileno(in), &st);
    char *buf = malloc(st.st_size);
    if(!buf) {
      printf("Can't allocate enough to read resource ¯\\_(ツ)_/¯\n");
      return 1;
    }
    if(fread(buf, 1, st.st_size, in) < st.st_size) {
      printf("Could not fully read input resource\n");
      return 1;
    }
    int src_len = st.st_size;
    int max_len = lzav_compress_bound_hi( src_len ); // Note another bound function!
    uint8_t* comp_buf = malloc( max_len );
    int comp_len = lzav_compress_hi( buf, comp_buf, src_len, max_len );

    if( comp_len == 0 && src_len != 0 ) {
      printf("couldn't compress\n");
      return 1;
    }
    printf("Including resource: %s (compressed %d -> %d)\n", argv[i], src_len, comp_len);
    fprintf(out,
            "static const char *_res2h_%d_name = \"%s\";\n"
            "static const size_t _res2h_%d_size[2] = { %lld, %d };\n"
            "static uint8_t *_res2h_%d_unpacked;\n"
            "static const uint8_t _res2h_%d_data[] = {\n",
            i, argv[i], i, st.st_size, comp_len, i, i);

    for(size_t i=0; i < comp_len; i++) {
      fprintf(out, "%d",  comp_buf[i]);
      if(i < comp_len-1) fprintf(out, ",");
      if(i > 0 && (i % 50) == 0) fprintf(out, "\n           ");
    }
    fprintf(out, " };\n");

  }
  fprintf(out, "bool get_resource(const char *name, size_t *size, uint8_t **data) {\n");
  for(int i=2; i < argc; i++) {
    fprintf(out,
            "  if(strcmp(name, \"%s\")==0) {\n"
            "    if(_res2h_%d_unpacked == NULL) {\n"
            // uncompress
            "      _res2h_%d_unpacked = malloc(_res2h_%d_size[0]);\n"
            "      if(!_res2h_%d_unpacked) return false;\n"
            "      int l = lzav_decompress(_res2h_%d_data, _res2h_%d_unpacked, _res2h_%d_size[1], _res2h_%d_size[0] );\n"
            "      if(l < 0) return 0;\n"
            "    }\n"
            "    *size = _res2h_%d_size[0];\n"
            "    *data = _res2h_%d_unpacked;\n"
            "    return true;\n"
            "  }\n" , argv[i], i, i, i, i, i, i, i, i, i ,i);
  }
  fprintf(out, "  return false;\n}");
  return 0;
}
