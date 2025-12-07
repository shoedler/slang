#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "file.h"
#include "native.h"
#include "object.h"
#include "value.h"
#include "vm.h"

static Value native_file_read(int argc, Value argv[]);
static Value native_file_write(int argc, Value argv[]);
static Value native_file_exists(int argc, Value argv[]);
static Value native_file_join_path(int argc, Value argv[]);

#define MODULE_NAME File

void native_register_file_module() {
  ObjObject* file_module = vm_make_module(NULL, STR(MODULE_NAME));
  define_value(&vm.modules, STR(MODULE_NAME), instance_value(file_module));

  define_value(&file_module->fields, "newl", str_value(copy_string(SLANG_ENV_NEWLINE, (int)strlen(SLANG_ENV_NEWLINE))));
  define_value(&file_module->fields, "sep", str_value(copy_string(SLANG_PATH_SEPARATOR_STR, (int)strlen(SLANG_PATH_SEPARATOR_STR))));

  define_native(&file_module->fields, "read", native_file_read, 1);
  define_native(&file_module->fields, "write", native_file_write, 2);
  define_native(&file_module->fields, "exists", native_file_exists, 1);
  define_native(&file_module->fields, "join_path", native_file_join_path, 2);
}

/**
 * MODULE_NAME.read(path: TYPENAME_STRING) -> TYPENAME_STRING
 * @brief Reads the content of a file at path (absolute) and returns it as a string. Throws an error if the file does not exist.
 */
static Value native_file_read(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  NATIVE_CHECK_ARG_AT(1, vm.str_class);

  const char* path = AS_CSTRING(argv[1]);

  if (!file_exists(path)) {
    vm_error("File '%s' does not exist.", path);
    return nil_value();
  }

  char* content = file_read_safe(path);
  if (content == NULL) {
    return nil_value();
  }

  Value result = str_value(copy_string(content, (int)strlen(content)));
  free(content);
  return result;
}

/**
 * MODULE_NAME.write(path: TYPENAME_STRING, content: TYPENAME_STRING) -> TYPENAME_BOOL
 * @brief Writes the 'content' to a file at 'path'. If the file does not exist, it will be created. Overwrites the file if it
 * exists. Returns VALUE_STR_TRUE on success, VALUE_STR_FALSE otherwise.
 */
static Value native_file_write(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  NATIVE_CHECK_ARG_AT(1, vm.str_class);
  NATIVE_CHECK_ARG_AT(2, vm.str_class);

  bool success = file_write(AS_CSTRING(argv[1]), AS_CSTRING(argv[2]));
  return bool_value(success);
}

/**
 * MODULE_NAME.exists(path: TYPENAME_STRING) -> TYPENAME_BOOL
 * @brief Returns VALUE_STR_TRUE if the file at 'path' exists, VALUE_STR_FALSE otherwise.
 */
static Value native_file_exists(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  NATIVE_CHECK_ARG_AT(1, vm.str_class);

  bool exists = file_exists(AS_CSTRING(argv[1]));
  return bool_value(exists);
}

/**
 * MODULE_NAME.join_path(path1: TYPENAME_STRING, path2: TYPENAME_STRING) -> TYPENAME_STRING | TYPENAME_NIL
 * @brief Joins two paths together and returns the result or TYPENAME_NIL, if joining failed. The paths are joined with the
 * system's path separator.
 */
static Value native_file_join_path(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  NATIVE_CHECK_ARG_AT(1, vm.str_class);
  NATIVE_CHECK_ARG_AT(2, vm.str_class);

  char* joined = file_join_path(AS_CSTRING(argv[1]), AS_CSTRING(argv[2]));
  if (joined == NULL) {
    return nil_value();
  }

  Value result = str_value(copy_string(joined, (int)strlen(joined)));
  free(joined);
  return result;
}
