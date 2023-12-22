#include "test.h"
#include "vm.h"

typedef enum { TYPE_PRINT, TYPE_COMPILE_ERROR, TYPE_RUNTIME_ERROR } ExpectType;

typedef struct {
  int line;
  ExpectType type;
  char* value;
} Expectation;

// Utility function to check if a file has the specified extension
bool has_extension(const wchar_t* file_name, const wchar_t* extension) {
  size_t len = _tcslen(file_name);
  size_t extLen = _tcslen(extension);
  return len > extLen && _tcscmp(file_name + len - extLen, extension) == 0;
}

// Recursive function to scan directory
void scan_tests_dir(const wchar_t* path,
                    wchar_t** file_paths,
                    int* count,
                    int max_files) {
  wchar_t search_path[MAX_PATH];
  WIN32_FIND_DATA find_file_data;
  HANDLE find_handle = INVALID_HANDLE_VALUE;

  // Prepare the path for search
  _stprintf(search_path, _T("%s\\*"), path);

  // Find the first file in the directory.
  find_handle = FindFirstFile(search_path, &find_file_data);
  if (find_handle == INVALID_HANDLE_VALUE) {
    return;  // Directory not found or empty
  }

  do {
    if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      // Found a directory, not a file
      if (_tcscmp(find_file_data.cFileName, _T(".")) != 0 &&
          _tcscmp(find_file_data.cFileName, _T("..")) != 0) {
        // Construct new path and recursively call this function
        wchar_t new_path[MAX_PATH];
        _stprintf(new_path, _T("%s\\%s"), path, find_file_data.cFileName);
        scan_tests_dir(new_path, file_paths, count, max_files);
      }
    } else {
      if (has_extension(find_file_data.cFileName, _T(".spec.sl"))) {
        // Store the .spec.sl file path
        wchar_t full_path[MAX_PATH];
        _stprintf(full_path, _T("%s\\%s"), path, find_file_data.cFileName);

        if (*count < max_files) {
          _tcscpy(file_paths[*count], full_path);
          (*count)++;
        }
      }
    }
  } while (FindNextFile(find_handle, &find_file_data) != 0 &&
           *count < max_files);

  FindClose(find_handle);
}

// Utility function to read a file
char* read_file(const wchar_t* path) {
  if (path == NULL) {
    WINTERNAL_ERROR(L"Cannot open NULL path \"%s\"", path);
    exit(74);
  }

  FILE* file = _wfopen(path, L"rb");
  if (file == NULL) {
    WINTERNAL_ERROR(L"Could not open file \"%s\"", path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(file_size + 1);
  if (buffer == NULL) {
    WINTERNAL_ERROR(L"Not enough memory to read \"%s\"\n", path);
    exit(74);
  }

  size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
  if (bytes_read < file_size) {
    WINTERNAL_ERROR(L"Could not read file \"%s\"\n", path);
    exit(74);
  }

  buffer[bytes_read] = '\0';

  fclose(file);
  return buffer;
}

// Utility to trim whitespace from a string
char* trim_whitespace(char* str) {
  while (isspace((unsigned char)*str))
    str++;
  if (*str == 0)
    return str;

  char* end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end))
    end--;
  *(end + 1) = 0;

  return str;
}

// Utility to count lines in a string
int count_lines(const char* str) {
  if (str == NULL) {
    return 0;
  }

  char* temp = strdup(str);
  if (temp == NULL) {
    INTERNAL_ERROR("Not enough memory to allocate temp");
    exit(70);
  }

  int line_count = 0;
  char* line = strtok(temp, "\r\n");

  while (line != NULL) {
    line_count++;
    line = strtok(NULL, "\r\n");
  }

  free(temp);
  return line_count;
}

// Utility to get the ExpectType from a tag
ExpectType get_expect_type(const char* tag) {
  if (strncmp(tag, "[ExpectRuntimeError", strlen("[ExpectRuntimeError")) == 0) {
    return TYPE_RUNTIME_ERROR;
  }
  if (strncmp(tag, "[ExpectCompileError", strlen("[ExpectCompileError")) == 0) {
    return TYPE_COMPILE_ERROR;
  }
  if (strncmp(tag, "[Expect", strlen("[Expect")) == 0) {
    return TYPE_PRINT;
  }
  INTERNAL_ERROR("Invalid ExpectType");
  exit(70);
}

// Utility to compare a single line with an Expectation
bool compare_line_with_expectation(const char* line,
                                   const Expectation* expectation) {
  return strcmp(line, expectation->value) == 0;
}

// Utility to parse expectations from a source string (multiline)
Expectation* parse_expectations(const char* input, int* count) {
  int line_count = count_lines(input);

  Expectation* expectations = malloc(sizeof(Expectation) * line_count);
  if (expectations == NULL) {
    INTERNAL_ERROR("Not enough memory to allocate expectations array");
    exit(70);
  }
  *count = 0;

  // Process each line
  char* temp = _strdup(input);
  if (temp == NULL) {
    INTERNAL_ERROR("Not enough memory to allocate temp");
    exit(70);
    return;
  }

  int line_no = 0;
  for (const char* line = strtok(temp, "\r\n"); line != NULL;
       line = strtok(NULL, "\r\n")) {
    line_no++;

    char* comment_start = strstr(line, "//");
    if (!comment_start) {
      continue;
    }

    comment_start += 2;  // Skip the "//"
    char* tag_start = strstr(comment_start, "[Expect");
    if (!tag_start) {
      continue;
    }

    char* tag_end = strchr(tag_start, ']');
    if (!tag_end) {
      continue;
    }

    *tag_end = '\0';  // Terminate the tag string
    ExpectType type = get_expect_type(tag_start);
    if (type == -1) {
      continue;
    }

    expectations[*count].line = line_no;
    expectations[*count].type = type;
    expectations[*count].value = _strdup(trim_whitespace(tag_end + 1));
    if (expectations[*count].value == NULL) {
      INTERNAL_ERROR(
          "Not enough memory to allocate expectations[*count].value");
      exit(70);
    }
    (*count)++;
  }

  free(temp);
  return expectations;
}

// Utility to compare a string with a set of expectations
const char** compare_string_with_expectations(const char* input,
                                              const Expectation* expectations,
                                              int num_expectations,
                                              int* num_differences) {
#define DIFF_LINE_SIZE 1024
  int line_count = count_lines(input);
  int diff_count = 0;
  int max_diffs = line_count > num_expectations ? line_count : num_expectations;
  const char** diff = malloc(DIFF_LINE_SIZE * (max_diffs + 1));
  if (diff == NULL) {
    INTERNAL_ERROR("Not enough memory to allocate diff array");
    exit(70);
  }

  char* temp = strdup(input);
  if (temp == NULL) {
    INTERNAL_ERROR("Not enough memory to allocate temp");
    exit(70);
  }

  // Compare exepecations array to input. Line by line
  char* line = strtok(temp, "\r\n");
  int line_no = 0;
  while (line != NULL && line_no < num_expectations) {
    if (!compare_line_with_expectation(line, &expectations[line_no])) {
      // Lines are different, format the string and add to diff
      char* diff_line = malloc(DIFF_LINE_SIZE);
      if (diff_line == NULL) {
        INTERNAL_ERROR("Not enough memory to allocate diff line");
        exit(70);
      }
      sprintf(diff_line,
              "on line %d: expected " ANSI_GREEN_STR(
                  "%s") ", but was " ANSI_RED_STR("%s") " in outfile",
              expectations[line_no].line, expectations[line_no].value, line);
      diff[diff_count++] = diff_line;
    }

    line = strtok(NULL, "\r\n");
    line_no++;
  }

  // If there are more lines in the output than expectations, we consider them
  // differences aswell
  if (line_count > num_expectations) {
    for (int i = line_no; i < line_count; i++) {
      char* diff_line = malloc(DIFF_LINE_SIZE);
      if (diff_line == NULL) {
        INTERNAL_ERROR("Not enough memory to allocate diff line");
        exit(70);
      }
      // Don't print the line number here, since it tells us nothing - it's the
      // line number in the output file, not the source file
      sprintf(diff_line, "unhandled output in outfile: " ANSI_YELLOW_STR("%s"),
              line);
      diff[diff_count++] = diff_line;
    }
  }

  // If there are more expectations in the source than in the output,
  // we consider them differences aswell
  if (line_count < num_expectations) {
    for (int i = line_no; i < num_expectations; i++) {
      char* diff_line = malloc(DIFF_LINE_SIZE);
      if (diff_line == NULL) {
        INTERNAL_ERROR("Not enough memory to allocate diff line");
        exit(70);
      }
      sprintf(diff_line,
              "unexhausted expectation on line %d: " ANSI_YELLOW_STR("%s"),
              expectations[i].line, expectations[i].value);
      diff[diff_count++] = diff_line;
    }
  }

  if (line_no) {
    // Finalize the array
    diff[diff_count] = NULL;
  }

  *num_differences = diff_count;

  free(temp);

  return diff;
#undef DIFF_LINE_SIZE
}

// Utility to run a test
bool run_test(const wchar_t* path) {
  init_vm();

  char* source = read_file(path);

  // Parse expectations from test source
  int num_expectations = 0;
  Expectation* expectations = parse_expectations(source, &num_expectations);

  // Create output file path based on file path for stdout
  wchar_t wout_file_path[MAX_PATH];
  _stprintf(wout_file_path, _T("%s.out"), path);

  char out_file_path[MAX_PATH];
  WideCharToMultiByte(CP_UTF8, 0, wout_file_path, -1, out_file_path, MAX_PATH,
                      NULL, NULL);

  // Redirect stdout to a file
  FILE* stream = freopen(out_file_path, "w", stdout);
  if (stream == NULL) {
    INTERNAL_ERROR("Could not redirect stdout to file");
    exit(70);
  }

  // Redirect stderr to the same file
  if (_dup2(_fileno(stream), _fileno(stderr)) != 0) {
    INTERNAL_ERROR("Could not redirect stderr to file");
    exit(70);
  }

  // Run the test
  InterpretResult result = interpret(source);

  free(source);
  free_vm();

  // Restore stdout
  fclose(stream);
  FILE* stdout_ = freopen("CONOUT$", "w", stdout);
  if (stdout_ == NULL) {
    INTERNAL_ERROR("Could not restore stdout");
    exit(70);
  }

  // Restore stderr
  if (_dup2(_fileno(stdout_), _fileno(stderr)) != 0) {
    INTERNAL_ERROR("Could not restore stderr");
    exit(70);
  }

  // Read & delete outfile (stdout log)
  wchar_t out_filepath[MAX_PATH];
  _stprintf(out_filepath, _T("%s%s"), path, L".out");
  char* out = read_file(out_filepath);
  if (_wremove(out_filepath) != 0) {
    INTERNAL_ERROR("Could not delete outfile");
    exit(70);
  }

  // Compare out with expected
  int num_differences = 0;
  bool passed = true;
  const char** differences = compare_string_with_expectations(
      out, expectations, num_expectations, &num_differences);
  if (num_differences > 0) {
    passed = false;
    printf(ANSI_RED_STR("Failed, %d diffs!\n"), num_differences);
    for (int i = 0; i < num_differences; i++) {
      printf("        #%d: %s\n", i + 1, differences[i]);
      free((void*)differences[i]);
    }
  }
  free(differences);

  // Check if there are any [ExpectRuntimeError] or [ExpectCompileError] tags
  // present in the expectations. If so, allow runtime and compile errors
  bool allow_runtime_errors = false;
  bool allow_compile_errors = false;
  for (int i = 0; i < num_expectations; i++) {
    if (expectations[i].type == TYPE_RUNTIME_ERROR) {
      allow_runtime_errors = true;
    }
    if (expectations[i].type == TYPE_COMPILE_ERROR) {
      allow_compile_errors = true;
    }
  }

  // Free expections
  for (int i = 0; i < num_expectations; i++) {
    free(expectations[i].value);
  }
  free(expectations);

  // Evaluate result
  if (!passed) {
    return passed;
  }

  switch (result) {
    case INTERPRET_OK:
      printf(ANSI_GREEN_STR("Passed!\n"));
      break;
    case INTERPRET_COMPILE_ERROR: {
      if (allow_compile_errors) {
        printf(ANSI_GREEN_STR("Passed!\n"));
        break;
      }
      passed = false;
      printf(ANSI_RED_STR("Failed, unexpected Compile Error!\n"));
      break;
    }
    case INTERPRET_RUNTIME_ERROR: {
      if (allow_runtime_errors) {
        printf(ANSI_GREEN_STR("Passed!\n"));
        break;
      }
      passed = false;
      printf(ANSI_RED_STR("Failed, unexpected Runtime Error!\n"));
      break;
    }
    default:
      passed = false;
      printf(ANSI_RED_STR("Failed due to Internal Error!\n"));
      INTERNAL_ERROR("Unhandled InterpretResult");
      break;
  }

  return passed;
}

// Utility to run all tests in a directory
void run_tests(const wchar_t* path) {
  wchar_t* test_file_paths[MAX_SPEC_FILES];   // Found test-file-paths. (*.spec.sl)
  int count = 0;                   // Number of files found

  for (int i = 0; i < MAX_SPEC_FILES; i++) {
    test_file_paths[i] = (wchar_t*)malloc(MAX_PATH * sizeof(wchar_t));
  }

  scan_tests_dir(path, test_file_paths, &count, MAX_SPEC_FILES);

  if (count == 0) {
    WINTERNAL_ERROR(L"No test files found in \"%s\"", path);
    exit(70);
  }

  if (count >= MAX_SPEC_FILES) {
    WINTERNAL_ERROR(L"Limit of %d test files reached", MAX_SPEC_FILES);
    exit(70);
  }

  printf("Running %d tests\n", count);

  // Runner
  int failed = 0;
  for (int i = 0; i < count; i++) {
    wprintf(L"    - running %s ", test_file_paths[i]);
    bool result = run_test(test_file_paths[i]);
    if (!result) {
      failed++;
    }
    free(test_file_paths[i]);
  }

  printf("Result: %d/%d tests passed\n", count - failed, count);
}
