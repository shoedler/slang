export const SLANG_PROJ_DIR = 'C:\\Projects\\slang';

export enum SlangBuildConfigs {
  Release = 'release',
  ReleaseProfiled = 'release-profiled',
  Debug = 'debug',
}

export enum SlangRunFlags {
  StressGc = '--stress-gc',
}

export enum SlangFileSuffixes {
  Slang = '.sl',
  Test = '.spec.sl',
}

export const BENCH_PRE_SUFFIX = '.bench';
export const BENCH_LOG_FILE = 'bench-log.json';

export enum SlangPaths {
  BinDir = SLANG_PROJ_DIR + '\\bin\\x64',
  BenchDir = SLANG_PROJ_DIR + '\\bench',
  TestDir = SLANG_PROJ_DIR + '\\test',
  SampleFile = SLANG_PROJ_DIR + '\\sample' + SlangFileSuffixes.Slang,
  RunnerSrcDir = SLANG_PROJ_DIR + '\\runner\\src',
}

export const LOCALE: Intl.LocalesArgument = 'de-CH';
