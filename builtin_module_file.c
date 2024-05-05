#include "builtin.h"
#include "common.h"
#include "file.h"
#include "vm.h"

BUILTIN_DECLARE_FN(read);
BUILTIN_DECLARE_FN(write);
BUILTIN_DECLARE_FN(exists);
BUILTIN_DECLARE_FN(join_path);

void register_builtin_file_module() {
  ObjObject* file_module = make_module(NULL, "File");
  define_obj(&vm.modules, "File", (Obj*)file_module);

  BUILTIN_REGISTER_FN(file_module, read, 1);
  BUILTIN_REGISTER_FN(file_module, write, 2);
  BUILTIN_REGISTER_FN(file_module, exists, 1);
  BUILTIN_REGISTER_FN(file_module, join_path, 2);
}

BUILTIN_FN_DOC(
    /* Fn Name     */ read,
    /* Arguments   */ DOC_ARG("path", TYPENAME_STRING),
    /* Return Type */ TYPENAME_STRING,
    /* Description */ "Reads the content of a file and returns it as a string. Throws an error if the file does not exist.");
BUILTIN_FN_IMPL(read) {
  UNUSED(argc);
  UNUSED(argv);
  BUILTIN_CHECK_ARG_AT(1, STRING);

  const char* path = AS_CSTRING(argv[1]);

  if (!file_exists(path)) {
    runtime_error("File '%s' does not exist.", path);
    return NIL_VAL;
  }

  char* content = read_file_safe(path);
  if (content == NULL) {
    return NIL_VAL;
  }

  Value result = OBJ_VAL(copy_string(content, (int)strlen(content)));
  free(content);
  return result;
}

BUILTIN_FN_DOC(
    /* Fn Name     */ write,
    /* Arguments   */ DOC_ARG("path", TYPENAME_STRING) DOC_ARG_SEP DOC_ARG("content", TYPENAME_STRING),
    /* Return Type */ TYPENAME_BOOL,
    /* Description */
    "Writes the 'content' to a file at 'path'. If the file does not exist, it will be created. Overwrites "
    "the file if it "
    "exists. Returns " VALUE_STR_TRUE " on success, " VALUE_STR_FALSE " otherwise.");
BUILTIN_FN_IMPL(write) {
  UNUSED(argc);
  UNUSED(argv);
  BUILTIN_CHECK_ARG_AT(1, STRING);
  BUILTIN_CHECK_ARG_AT(2, STRING);

  bool success = write_file(AS_CSTRING(argv[1]), AS_CSTRING(argv[2]));
  return BOOL_VAL(success);
}

BUILTIN_FN_DOC(
    /* Fn Name     */ exists,
    /* Arguments   */ DOC_ARG("path", TYPENAME_STRING),
    /* Return Type */ TYPENAME_BOOL,
    /* Description */
    "Returns " VALUE_STR_TRUE " if the file at 'path' exists, " VALUE_STR_FALSE " otherwise.");
BUILTIN_FN_IMPL(exists) {
  UNUSED(argc);
  UNUSED(argv);
  BUILTIN_CHECK_ARG_AT(1, STRING);

  bool exists = file_exists(AS_CSTRING(argv[1]));
  return BOOL_VAL(exists);
}

BUILTIN_FN_DOC(
    /* Fn Name     */ join_path,
    /* Arguments   */ DOC_ARG("path1", TYPENAME_STRING) DOC_ARG_SEP DOC_ARG("path2", TYPENAME_STRING),
    /* Return Type */ TYPENAME_STRING | TYPENAME_NIL,
    /* Description */
    "Joins two paths together and returns the result or " STR(TYPENAME_NIL) " if joining failed. The paths are joined with the system's path "
    "separator.");
BUILTIN_FN_IMPL(join_path) {
  UNUSED(argc);
  UNUSED(argv);
  BUILTIN_CHECK_ARG_AT(1, STRING);
  BUILTIN_CHECK_ARG_AT(2, STRING);

  char* joined = join_path(AS_CSTRING(argv[1]), AS_CSTRING(argv[2]));
  if (joined == NULL) {
    return NIL_VAL;
  }

  Value result = OBJ_VAL(copy_string(joined, (int)strlen(joined)));
  free(joined);
  return result;
}