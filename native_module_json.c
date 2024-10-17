#include "builtin.h"
#include "common.h"
#include "file.h"
#include "memory.h"
#include "strbuf.h"
#include "vm.h"

static Value native_json_parse(int argc, Value argv[]);
static Value native_json_stringify(int argc, Value argv[]);

static ObjString* stringify_value(Value value, int indent_size, int current_indent, bool produce_string);

#define MODULE_NAME Json

void register_native_json_module() {
  ObjObject* gc_module = make_module(NULL, STR(MODULE_NAME));
  define_value(&vm.modules, STR(MODULE_NAME), instance_value(gc_module));

  define_native(&gc_module->fields, "parse", native_json_parse, 1);
  define_native(&gc_module->fields, "stringify", native_json_stringify, 2);
}

/**
 * MODULE_NAME.parse(raw: TYPENAME_STR) -> TYPENAME_OBJ | TYPENAME_NIL
 * @brief Parses [raw] as a JSON string. Returns the parsed value or TYPENAME_NIL if the TYPENAME_STR is invalid.
 */
static Value native_json_parse(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  return nil_value();
}

/**
 * MODULE_NAME.stringify(val: TYPENAME_VALUE, indent?: TYPENAME_INT) -> TYPENAME_STR
 * @brief Converts [val] to a JSON string. If [indent] is greater than 0, the string will be indented by [indent] spaces.
 */
static Value native_json_stringify(int argc, Value argv[]) {
  UNUSED(argc);
  NATIVE_CHECK_ARG_AT(2, vm.int_class)

  ObjString* json_str = stringify_value(argv[1], (int)argv[2].as.integer, 0, false /* produce_string */);
  if (vm.flags & VM_FLAG_HAS_ERROR) {
    return nil_value();
  }

  return str_value(json_str);
}

#define JSON_OBJ_START "{"
#define JSON_OBJ_END "}"
#define JSON_OBJ_DELIM ","
#define JSON_OBJ_SEPARATOR ":"

#define JSON_ARRAY_START "["
#define JSON_ARRAY_END "]"
#define JSON_ARRAY_DELIM ","

#define JSON_QUOTE_CHAR '"'
#define JSON_ESCAPE_CHAR '\\'

#define JSON_NEWLINE "\n"
#define JSON_INDENT " "

static ObjString* quotify(ObjString* raw) {
  push(str_value(raw));  // GC Protection

  // Add quotes
  char* heap_chars = (char*)malloc(raw->length + 3);  // +3 for the quotes and null-terminator

  heap_chars[0] = JSON_QUOTE_CHAR;
  memcpy(heap_chars + 1, raw->chars, raw->length);
  heap_chars[raw->length + 1] = JSON_QUOTE_CHAR;
  heap_chars[raw->length + 2] = '\0';

  ObjString* quoted = take_string(heap_chars, raw->length + 2);  // Only +2 required here
  pop();                                                         // Raw string

  return quoted;
}

static ObjString* sanitize_string(ObjString* raw) {
  push(str_value(raw));  // GC Protection

  const char* input = raw->chars;

  size_t output_size = raw->length * 2 + 1;  // Maximum possible size
  char* output       = malloc(output_size);
  if (!output) {
    return NULL;
  }

  char* out_ptr = output;

  for (int i = 0; i < raw->length; ++i) {
    char c = input[i];

    if (c == '\\') {
      if (i + 1 < raw->length) {
        char next_c = input[i + 1];
        if (next_c == 'n' || next_c == '\'' || next_c == '"') {
          // Already escaped, copy both characters
          *out_ptr++ = c;
          *out_ptr++ = next_c;
          ++i;
          continue;
        }
      }
      // Copy the backslash as is
      *out_ptr++ = c;
      continue;
    }

    if (c == '\n') {
      // Escape newline
      *out_ptr++ = '\\';
      *out_ptr++ = 'n';
    } else if (c == '\'' || c == '"') {
      // Escape quotes
      *out_ptr++ = '\\';
      *out_ptr++ = c;
    } else {
      *out_ptr++ = c;
    }
  }

  *out_ptr = '\0';  // Null-terminate the output string

  ObjString* sanitized = copy_string(output, (int)(out_ptr - output));

  free(output);
  pop();  // Raw string

  return sanitized;
}

static ObjString* stringify_obj(ObjObject* object, int indent, int current_indent) {
  StringBuffer buf;
  init_strbuf(&buf);
  int processed = 0;  // Keep track of how many non-EMPTY fields we've processed to know when to skip the
                      // last delimiter

  // Start the obj string
  strbuf_append(&buf, JSON_OBJ_START, STR_LEN(JSON_OBJ_START));
  for (int i = 0; i < object->fields.capacity; i++) {
    if (is_empty_internal(object->fields.entries[i].key)) {
      continue;
    }

    // Just use the to_str method on the key - we cannot have a non-string as a key in JSON.
    ObjString* key_str =
        stringify_value(object->fields.entries[i].key, indent, current_indent + indent, true /* produce_string */);
    if (vm.flags & VM_FLAG_HAS_ERROR) {
      key_str = copy_string("???", 3);  // Fallback, generates invalid JSON by design
    }
    push(str_value(key_str));  //  GC Protection

    // The value however, needs to be stringified properly
    ObjString* value_str =
        stringify_value(object->fields.entries[i].value, indent, current_indent + indent, false /* produce_string */);
    if (vm.flags & VM_FLAG_HAS_ERROR) {
      value_str = copy_string("???", 3);  // Fallback, generates invalid JSON by design
    }

    pop();  // Key str

    // Add a newline and indentation if we're indenting
    if (indent > 0) {
      if (processed > 0) {
        strbuf_append(&buf, JSON_OBJ_DELIM, STR_LEN(JSON_OBJ_DELIM));
      }
      strbuf_append(&buf, JSON_NEWLINE, STR_LEN(JSON_NEWLINE));
      for (int i = 0; i < current_indent + indent; i++) {
        strbuf_append(&buf, JSON_INDENT, STR_LEN(JSON_INDENT));
      }
    }

    // Append the strings
    strbuf_append(&buf, key_str->chars, key_str->length);
    strbuf_append(&buf, JSON_OBJ_SEPARATOR, STR_LEN(JSON_OBJ_SEPARATOR));
    strbuf_append(&buf, value_str->chars, value_str->length);

    processed++;
  }

  // End the obj string
  if (processed > 0) {
    strbuf_append(&buf, JSON_NEWLINE, STR_LEN(JSON_NEWLINE));
    for (int i = 0; i < current_indent; i++) {
      strbuf_append(&buf, JSON_INDENT, STR_LEN(JSON_INDENT));
    }
  }
  strbuf_append(&buf, JSON_OBJ_END, STR_LEN(JSON_OBJ_END));

  return strbuf_take_string(&buf);
}

static ObjString* stringify_seq(ObjSeq* seq, int indent, int current_indent) {
  StringBuffer buf;
  init_strbuf(&buf);
  int processed = 0;  // Keep track of how many non-EMPTY fields we've processed to know when to skip the
                      // last delimiter

  // Start the array string
  strbuf_append(&buf, JSON_ARRAY_START, STR_LEN(JSON_ARRAY_START));
  for (int i = 0; i < seq->items.count; i++) {
    Value item = seq->items.values[i];

    ObjString* item_str = stringify_value(item, indent, current_indent + indent, false /* produce_string */);
    if (vm.flags & VM_FLAG_HAS_ERROR) {
      item_str = copy_string("???", 3);  // Fallback, generates invalid JSON by design
    }

    // Add a newline and indentation if we're indenting
    if (indent > 0) {
      if (processed > 0) {
        strbuf_append(&buf, JSON_ARRAY_DELIM, STR_LEN(JSON_ARRAY_DELIM));
      }
      strbuf_append(&buf, JSON_NEWLINE, STR_LEN(JSON_NEWLINE));
      for (int i = 0; i < current_indent + indent; i++) {
        strbuf_append(&buf, JSON_INDENT, STR_LEN(JSON_INDENT));
      }
    } else if (processed > 0) {
      strbuf_append(&buf, JSON_ARRAY_DELIM, STR_LEN(JSON_ARRAY_DELIM));
    }

    // Append the strings
    strbuf_append(&buf, item_str->chars, item_str->length);

    processed++;
  }

  // End the array string
  if (processed > 0 && indent > 0) {
    strbuf_append(&buf, JSON_NEWLINE, STR_LEN(JSON_NEWLINE));
    for (int i = 0; i < current_indent; i++) {
      strbuf_append(&buf, JSON_INDENT, STR_LEN(JSON_INDENT));
    }
  }
  strbuf_append(&buf, JSON_ARRAY_END, STR_LEN(JSON_ARRAY_END));

  return strbuf_take_string(&buf);
}

static ObjString* stringify_value(Value value, int indent_size, int current_indent, bool is_key) {
  ObjString* str_value = NULL;

  if (is_bool(value)) {
    const char* json_str = value.as.boolean ? "true" : "false";  // Json allows 'true' and 'false' as literals, without quotes.
    str_value            = copy_string(json_str, (int)strlen(json_str));
  } else if (is_nil(value)) {
    const char* json_str = "null";  // ... same goes for 'null',
    str_value            = copy_string(json_str, (int)strlen(json_str));
  } else if (is_int(value))
    str_value = integer_to_string(value.as.integer);  // ... and integers,
  else if (is_float(value))
    str_value = double_to_string(value.as.float_);  // ... and floats.
  else if (is_obj(value)) {
    // If we stringify an object for a key, we don't want to pretty-print it.
    int _indent_size    = is_key ? 0 : indent_size;
    int _current_indent = is_key ? 0 : current_indent;
    str_value           = stringify_obj(AS_OBJECT(value), _indent_size, _current_indent);  // Objects are handled separately.
  } else if (is_seq(value)) {
    // If we stringify a sequence for a key, we don't want to pretty-print it.
    int _indent_size    = is_key ? 0 : indent_size;
    int _current_indent = is_key ? 0 : current_indent;
    str_value           = stringify_seq(AS_SEQ(value), _indent_size, _current_indent);  // Seqs are handled separately
  } else if (is_str(value)) {
    str_value = sanitize_string(AS_STR(value));  // Strings are always quoted, so we can directly return the quoted string.
    str_value = quotify(str_value);
    return str_value;
  } else {
    // Every other type just gets to_str'd and wrapped in quotes.
    push(value);
    str_value = AS_STR(exec_callable(fn_value(value.type->__to_str), 0));
    str_value = sanitize_string(str_value);
    str_value = quotify(str_value);
    return str_value;  // Therefore, we can directly return the quoted resulting string.
  }

  // Every value can be used as a key
  if (is_key) {
    str_value = sanitize_string(str_value);
    return quotify(str_value);
  }

  return str_value;
}
