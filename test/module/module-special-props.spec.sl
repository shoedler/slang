// SP_PROP_FILE_PATH
print __file_path.split("\\")[-2] // [expect] module

// SP_PROP_MODULE_NAME 
print __module_name // [expect] main

// This is not supposed to be allowed:
__module_name = "new_module"
print __module_name // [expect] new_module