#include "native.h"
#include "common.h"
#include "value.h"
#include "vm.h"

bool native_set_prop_not_supported(Value receiver, ObjString* name, Value value) {
  UNUSED(name);
  UNUSED(value);
  vm_error("Type %s does not support property-set access.", receiver.type->name->chars);
  return false;
}

bool native_get_subs_not_supported(Value receiver, Value index, Value* result) {
  UNUSED(index);
  UNUSED(result);
  vm_error("Type %s does not support get-subscripting.", receiver.type->name->chars);
  return false;
}

bool native_set_subs_not_supported(Value receiver, Value index, Value value) {
  UNUSED(index);
  UNUSED(value);
  vm_error("Type %s does not support set-subscripting.", receiver.type->name->chars);
  return false;
}

bool native_equals_not_supported(Value self, Value other) {
  UNUSED(other);
  vm_error("Type %s does not support equality-comparison.", self.type->name->chars);
  return false;
}

uint64_t native_hash_not_supported(Value self) {
  vm_error("Type %s does not support hashing.", self.type->name->chars);
  return false;
}

bool native_default_obj_equals(Value self, Value other) {
  return self.type == other.type && self.as.obj->hash == other.as.obj->hash;
}

uint64_t native_default_obj_hash(Value self) {
  return self.as.obj->hash;
}

#define NATIVE_SP_METHOD_NOT_SUPPORTED_BODY(method)                                     \
  UNUSED(argc);                                                                         \
  vm_error("Type %s does not support \"" STR(method) "\".", argv[0].type->name->chars); \
  return nil_value();

Value native___has_not_supported(int argc, Value argv[]) {
  NATIVE_SP_METHOD_NOT_SUPPORTED_BODY(SP_METHOD_HAS);
}
Value native___slice_not_supported(int argc, Value argv[]) {
  NATIVE_SP_METHOD_NOT_SUPPORTED_BODY(SP_METHOD_SLICE);
}
Value native___add_not_supported(int argc, Value argv[]) {
  NATIVE_SP_METHOD_NOT_SUPPORTED_BODY(SP_METHOD_ADD);
}
Value native___sub_not_supported(int argc, Value argv[]) {
  NATIVE_SP_METHOD_NOT_SUPPORTED_BODY(SP_METHOD_SUB);
}
Value native___mul_not_supported(int argc, Value argv[]) {
  NATIVE_SP_METHOD_NOT_SUPPORTED_BODY(SP_METHOD_MUL);
}
Value native___div_not_supported(int argc, Value argv[]) {
  NATIVE_SP_METHOD_NOT_SUPPORTED_BODY(SP_METHOD_DIV);
}
Value native___mod_not_supported(int argc, Value argv[]) {
  NATIVE_SP_METHOD_NOT_SUPPORTED_BODY(SP_METHOD_MOD);
}
Value native___lt_not_supported(int argc, Value argv[]) {
  NATIVE_SP_METHOD_NOT_SUPPORTED_BODY(SP_METHOD_LT);
}
Value native___gt_not_supported(int argc, Value argv[]) {
  NATIVE_SP_METHOD_NOT_SUPPORTED_BODY(SP_METHOD_GT);
}
Value native___lteq_not_supported(int argc, Value argv[]) {
  NATIVE_SP_METHOD_NOT_SUPPORTED_BODY(SP_METHOD_LTEQ);
}
Value native___gteq_not_supported(int argc, Value argv[]) {
  NATIVE_SP_METHOD_NOT_SUPPORTED_BODY(SP_METHOD_GTEQ);
}

#undef NATIVE_SP_METHOD_NOT_SUPPORTED_BODY
