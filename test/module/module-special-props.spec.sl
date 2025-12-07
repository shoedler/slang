import File
// SP_PROP_FILE_PATH
print __file_path.split(File.sep)[-2] // [expect] module

// SP_PROP_MODULE_NAME 
print __module_name // [expect] main
