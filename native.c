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