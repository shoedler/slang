import path from 'node:path';
import { fileURLToPath } from 'node:url';

// Detect project directory relative to this file (platform-agnostic)
const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
export const SLANG_PROJ_DIR = path.resolve(__dirname, '..', '..');

// Detect platform-specific binary extension
const BINARY_EXT = process.platform === 'win32' ? '.exe' : '';

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
  Profile = '.tpp.sl',
  Binary = '.slang' + BINARY_EXT,
}

export const BENCH_PRE_SUFFIX = '.bench';
export const BENCH_LOG_FILE = 'bench-log.json';
export const PGO_BENCH_LOG_FILE = 'pgo-bench-log.json';

export class SlangPaths {
  static readonly BinDir = path.join(SLANG_PROJ_DIR, 'bin', 'x64');
  static readonly BenchDir = path.join(SLANG_PROJ_DIR, 'bench');
  static readonly TestDir = path.join(SLANG_PROJ_DIR, 'test');
  static readonly ProfileDir = path.join(SLANG_PROJ_DIR, 'profile');
  static readonly SampleFile = path.join(SLANG_PROJ_DIR, 'sample' + SlangFileSuffixes.Slang);
  static readonly ProfileFile = path.join(SlangPaths.ProfileDir, 'profile' + SlangFileSuffixes.Slang);
  static readonly RunnerSrcDir = path.join(SLANG_PROJ_DIR, 'runner', 'src');
}

export const LOCALE: Intl.LocalesArgument = 'de-CH';
