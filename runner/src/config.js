export const MSBUILD_EXE =
  '"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\msbuild.exe"';
export const SLANG_PROJ_DIR = "C:\\Projects\\slang";

export const SLANG_BIN_DIR = SLANG_PROJ_DIR + '\\x64';
export const SLANG_BENCH_DIR = SLANG_PROJ_DIR + '\\bench';
export const SLANG_TEST_DIR = SLANG_PROJ_DIR + '\\test';
export const SLANG_SAMPLE_FILE = SLANG_PROJ_DIR + '\\sample.sl';

export const SLANG_BENCH_SUFFIX = '.bench.sl';
export const SLANG_TEST_SUFFIX = '.spec.sl';
export const BENCH_LOG_FILE = 'bench-log.json';

export const BUILD_CONFIG_RELEASE = 'Release';
export const BUILD_CONFIG_DEBUG = 'Debug';

/**
 * The locale to use for formatting numbers
 * @type {Intl.LocalesArgument}
 */
export const LOCALE = 'de-CH';
