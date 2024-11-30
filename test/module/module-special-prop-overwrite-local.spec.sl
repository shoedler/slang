// [exit] 2
// SP_PROP_MODULE_NAME 
{
  __module_name = "new_module"
  print __module_name // [expect-error] Compile error at line 4 at '__module_name': Cannot assign to global reserved property '__module_name'.
}