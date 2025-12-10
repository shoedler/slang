#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include "common.h"
#include "native.h"
#include "object.h"
#include "value.h"
#include "vm.h"

static Value native_math_abs(int argc, Value argv[]);
static Value native_math_ceil(int argc, Value argv[]);
static Value native_math_floor(int argc, Value argv[]);
static Value native_math_pow(int argc, Value argv[]);
static Value native_math_xor(int argc, Value argv[]);
static Value native_math_shl(int argc, Value argv[]);
static Value native_math_shr(int argc, Value argv[]);
static Value native_math_bor(int argc, Value argv[]);
static Value native_math_band(int argc, Value argv[]);
static Value native_math_sqrt(int argc, Value argv[]);
// static Value native_math_max(int argc, Value argv[]);
// static Value native_math_min(int argc, Value argv[]);

#define MODULE_NAME Math

void native_register_math_module() {
  ObjObject* math_module = vm_make_module(NULL, STR(MODULE_NAME));
  define_value(&vm.modules, STR(MODULE_NAME), instance_value(math_module));

  define_native(&math_module->fields, "abs", native_math_abs, 1);
  define_native(&math_module->fields, "ceil", native_math_ceil, 1);
  define_native(&math_module->fields, "floor", native_math_floor, 1);
  define_native(&math_module->fields, "pow", native_math_pow, 2);
  define_native(&math_module->fields, "xor", native_math_xor, 2);
  define_native(&math_module->fields, "shl", native_math_shl, 2);
  define_native(&math_module->fields, "shr", native_math_shr, 2);
  define_native(&math_module->fields, "bor", native_math_bor, 2);
  define_native(&math_module->fields, "band", native_math_band, 2);
  define_native(&math_module->fields, "sqrt", native_math_sqrt, 1);
  // define_native(&math_module->fields, "max", native_math_max, -1);
  // define_native(&math_module->fields, "min", native_math_min, -1);
}

/**
 * MODULE_NAME.abs(num: TYPENAME_NUM) -> TYPENAME_NUM
 * @brief Returns the absolute value of 'num'.
 */
static Value native_math_abs(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  NATIVE_CHECK_ARG_AT_INHERITS(1, vm.num_class);

  if (is_int(argv[1])) {
    return int_value(llabs(argv[1].as.integer));
  }

  return float_value(fabs(argv[1].as.float_));
}

/**
 * MODULE_NAME.ceil(num: TYPENAME_NUM) -> TYPENAME_INT
 * @brief Returns the smallest integer greater than or equal to 'num'.
 */
static Value native_math_ceil(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  NATIVE_CHECK_ARG_AT_INHERITS(1, vm.num_class);

  if (is_int(argv[1])) {
    return argv[1];
  }

  return int_value((long long)ceil(argv[1].as.float_));
}

/**
 * MODULE_NAME.floor(num: TYPENAME_NUM) -> TYPENAME_INT
 * @brief Returns the largest integer less than or equal to 'num'.
 */
static Value native_math_floor(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  NATIVE_CHECK_ARG_AT_INHERITS(1, vm.num_class);

  if (is_int(argv[1])) {
    return argv[1];
  }

  return int_value((long long)floor(argv[1].as.float_));
}

/**
 * MODULE_NAME.pow(base: TYPENAME_NUM, exponent: TYPENAME_NUM) -> TYPENAME_NUM
 * @brief Returns 'base' raised to the power of 'exponent'.
 */
static Value native_math_pow(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  NATIVE_CHECK_ARG_AT_INHERITS(1, vm.num_class);
  NATIVE_CHECK_ARG_AT_INHERITS(2, vm.num_class);

  if (is_int(argv[1]) && is_int(argv[2])) {
    return int_value(llround(pow(argv[1].as.integer, argv[2].as.integer)));
  }

  return float_value(pow(argv[1].as.float_, argv[2].as.float_));
}

/**
 * MODULE_NAME.xor(num1: TYPENAME_INT, num2: TYPENAME_INT) -> TYPENAME_INT
 * @brief Returns the bitwise XOR of 'num1' and 'num2'.
 */
static Value native_math_xor(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  NATIVE_CHECK_ARG_AT_INHERITS(1, vm.int_class);
  NATIVE_CHECK_ARG_AT_INHERITS(2, vm.int_class);

  return int_value(argv[1].as.integer ^ argv[2].as.integer);
}

/**
 * MODULE_NAME.shl(num: TYPENAME_INT, shift: TYPENAME_INT) -> TYPENAME_INT
 * @brief Returns 'num' shifted left by 'shift' bits.
 */
static Value native_math_shl(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  NATIVE_CHECK_ARG_AT_INHERITS(1, vm.int_class);
  NATIVE_CHECK_ARG_AT_INHERITS(2, vm.int_class);

  return int_value(argv[1].as.integer << argv[2].as.integer);
}

/**
 * MODULE_NAME.shr(num: TYPENAME_INT, shift: TYPENAME_INT) -> TYPENAME_INT
 * @brief Returns 'num' shifted right by 'shift' bits.
 */
static Value native_math_shr(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  NATIVE_CHECK_ARG_AT_INHERITS(1, vm.int_class);
  NATIVE_CHECK_ARG_AT_INHERITS(2, vm.int_class);

  return int_value(argv[1].as.integer >> argv[2].as.integer);
}

/**
 * MODULE_NAME.bor(num1: TYPENAME_INT, num2: TYPENAME_INT) -> TYPENAME_INT
 * @brief Returns the bitwise OR of 'num1' and 'num2'.
 */
static Value native_math_bor(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  NATIVE_CHECK_ARG_AT_INHERITS(1, vm.int_class);
  NATIVE_CHECK_ARG_AT_INHERITS(2, vm.int_class);

  return int_value(argv[1].as.integer | argv[2].as.integer);
}

/**
 * MODULE_NAME.band(num1: TYPENAME_INT, num2: TYPENAME_INT) -> TYPENAME_INT
 * @brief Returns the bitwise AND of 'num1' and 'num2'.
 */
static Value native_math_band(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  NATIVE_CHECK_ARG_AT_INHERITS(1, vm.int_class);
  NATIVE_CHECK_ARG_AT_INHERITS(2, vm.int_class);

  return int_value(argv[1].as.integer & argv[2].as.integer);
}

/**
 * MODULE_NAME.sqrt(num: TYPENAME_NUM) -> TYPENAME_FLOAT
 * @brief Returns the square root of num
 */
static Value native_math_sqrt(int argc, Value argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  NATIVE_CHECK_ARG_AT_INHERITS(1, vm.num_class);
  if (is_int(argv[1])) {
    return float_value(sqrt(argv[1].as.integer));
  }

  return float_value(sqrt(argv[1].as.float_));
}

// /**
//  * MODULE_NAME.max(num1: TYPENAME_NUM, ...nums: TYPENAME_NUM) -> TYPENAME_NUM
//  * @brief Returns the largest number from the provided arguments.
//  */
// static Value native_math_max(int argc, Value argv[]) {
//   if (argc == 1) {
//     NATIVE_CHECK_ARG_AT_INHERITS(1, vm.num_class);
//     return argv[1];
//   }

//   Value max = argv[1];
//   for (int i = 2; i <= argc; i++) {
//     NATIVE_CHECK_ARG_AT_INHERITS(i, vm.num_class);
//     if (value_is_greater(argv[i], max)) {
//       max = argv[i];
//     }
//   }

//   return max;
// }
