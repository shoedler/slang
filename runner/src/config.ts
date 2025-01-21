export const SLANG_PROJ_DIR = 'C:\\Projects\\slang';

export enum SlangBuildConfigs {
  Release = 'release',
  ReleaseProfiled = 'release-profiled',
  Debug = 'debug',
}

export enum SlangDefines {
  EnableColorOutput = 'SLANG_ENABLE_COLOR_OUTPUT',
}

export enum SlangRunFlags {
  StressGc = '--stress-gc',
  DisableWarnings = '--no-warn',
}

export enum SlangFileSuffixes {
  Slang = '.sl',
  Test = '.spec.sl',
  Profile = '.profile.sl',
  Binary = '.slang.exe',
}

export const BENCH_PRE_SUFFIX = '.bench';
export const BENCH_LOG_FILE = 'bench-log.json';
export const PGO_BENCH_LOG_FILE = 'pgo-bench-log.json';

export enum SlangPaths {
  BinDir = SLANG_PROJ_DIR + '\\bin\\x64',
  BenchDir = SLANG_PROJ_DIR + '\\bench',
  TestDir = SLANG_PROJ_DIR + '\\test',
  ProfileDir = SLANG_PROJ_DIR + '\\profile',
  SampleFile = SLANG_PROJ_DIR + '\\sample' + SlangFileSuffixes.Slang,
  ProfileFile = SlangPaths.ProfileDir + '\\profile' + SlangFileSuffixes.Slang,
  RunnerSrcDir = SLANG_PROJ_DIR + '\\runner\\src',
}

export const LOCALE: Intl.LocalesArgument = 'de-CH';
