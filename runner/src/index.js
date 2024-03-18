import { runBenchmarks, serveResults } from './bench.js';
import {
  BUILD_CONFIG_DEBUG,
  BUILD_CONFIG_RELEASE,
  SLANG_PROJ_DIR,
  SLANG_SAMPLE_FILE,
  SLANG_TEST_SUFFIX,
} from './config.js';
import { runTests } from './test.js';
import { abort, buildSlangConfig, info, ok, runSlangFile, separator, warn } from './utils.js';
import { watch } from './watch.js';

// Process arguments. This is a simple command line interface for running benchmarks and tests
// process.argv contains
// - 0: node executable path
// - 1: script path
// - 2: command
// - 3: options
// - 4: options
// - ...
const cmd = process.argv[2];
let options = process.argv.slice(3);

// Consume (remove) an option from the options array an return it's value. If the option is not found, return the default value
const consumeOption = (opt, defaultValue) => {
  const index = options.indexOf(opt);
  if (index < 0) {
    return defaultValue;
  }
  return options.splice(index, 1)[0];
};

// Validate that all options have been consumed, otherwise abort with an error
const validateOptions = () => {
  if (options.length > 0) {
    abort('Unknown options', options.join(', '));
  }
};

const hint = [
  'Available commands & options:',
  '  - bench           Run benchmarks',
  '  - test            Run tests (.spec.sl files)',
  '    - update-files  Update test files with new expectations',
  '    - <pattern>     Run tests that match the regex pattern',
  '  - watch-sample    Watch sample file',
  '  - watch-tests     Watch test files',
  '    - <pattern>     Watch tests that match the regex pattern',
  '  - serve-results   Serve benchmark results',
];

switch (cmd) {
  case 'bench': {
    const configs = [BUILD_CONFIG_DEBUG, BUILD_CONFIG_RELEASE];
    validateOptions();

    for (const config of configs) {
      await buildSlangConfig(config);
    }
    await runBenchmarks(configs);
    break;
  }
  case 'serve-results': {
    validateOptions();
    await serveResults();
    break;
  }
  case 'test': {
    const config = BUILD_CONFIG_RELEASE;
    const doUpdateFiles = Boolean(consumeOption('update-files', false));
    const testNamePattern = options.pop() || '.*';
    validateOptions();

    await buildSlangConfig(config);
    await runTests(config, undefined, doUpdateFiles, testNamePattern);
    break;
  }
  case 'watch-sample': {
    const config = BUILD_CONFIG_RELEASE;
    const sampleFilePath = SLANG_SAMPLE_FILE;
    validateOptions();

    watch(
      SLANG_PROJ_DIR,
      { recursive: true },
      filename =>
        filename.endsWith('.c') || filename.endsWith('.h') || sampleFilePath.endsWith(filename),
      async signal => {
        const didBuild = await buildSlangConfig(config, signal, false /* don't abort on error */);
        if (!didBuild) {
          return;
        }

        info('Running slang file', sampleFilePath);
        const { exitCode, rawOutput } = await runSlangFile(sampleFilePath, config, signal, true);
        console.clear();
        separator();
        console.log(rawOutput);
        separator();
        if (exitCode === 0) {
          ok('Ran with 0 exit code');
        } else {
          warn('Ran with non-zero exit code', exitCode);
        }
      },
    );
    break;
  }
  case 'watch-tests': {
    const config = BUILD_CONFIG_RELEASE;
    const testNamePattern = options.pop() || '.*';
    validateOptions();

    watch(
      SLANG_PROJ_DIR,
      { recursive: true },
      filename =>
        filename.endsWith('.c') ||
        filename.endsWith('.h') ||
        (filename.endsWith(SLANG_TEST_SUFFIX) && new RegExp(testNamePattern).test(filename)),
      async signal => {
        const didBuild = await buildSlangConfig(config, signal, false /* don't abort on error */);
        if (!didBuild) {
          return;
        }

        await runTests(config, signal, false, testNamePattern);
      },
    );
    break;
  }
  default: {
    abort('Unknown command', hint.join('\n'));
  }
}
