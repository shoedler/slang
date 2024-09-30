import { runBenchmarks, serveResults } from './bench.js';
import {
  BUILD_CONFIG_RELEASE,
  BUILD_CONFIG_RELEASE_PROFILED,
  SLANG_PROJ_DIR,
  SLANG_SAMPLE_FILE,
  SLANG_TEST_SUFFIX,
} from './config.js';
import { runTests } from './test.js';
import {
  abort,
  buildSlangConfig,
  info,
  runSlangFile,
  separator,
  testFeatureFlag,
  warn,
} from './utils.js';
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

// Check if the GC stress flag is set to the expected value. If not, print a warning
const checkFeatureFlagsForTests = async () => {
  const gcStressEnabled = await testFeatureFlag('DEBUG_STRESS_GC'); // Enable stress GC for tests
  if (!gcStressEnabled) {
    warn(
      "GC stress mode is disabled. Should be enabled to ensure that the GC only collects what it's supposed to collect.",
    );
  }

  const ansiColorsEnabled = await testFeatureFlag('ENABLE_COLOR_OUTPUT');
  if (ansiColorsEnabled) {
    abort('ANSI colors are enabled. Must be disabled for tests to ensure consistent results.');
  }
};

const hint = [
  'Available commands & options:',
  '  - bench           Run benchmarks (debug & release) and serve results',
  '    - serve         Only serve benchmark results',
  '    - no-serve      Run benchmarks without serving results',
  '    - <pattern>     Run language that matches the regex pattern',
  '  - sample          Run sample file (sample.sl)',
  '  - test            Run tests (.spec.sl files)',
  '    - update-files  Update test files with new expectations',
  '    - <pattern>     Run tests that match the regex pattern',
  '  - watch-sample    Watch sample file (sample.sl)',
  '  - watch-test      Watch test files',
  '    - <pattern>     Watch tests that match the regex pattern',
  '',
  'Note: If not specified, the default configuration is release',
];

switch (cmd) {
  case 'bench': {
    const doOnlyServe = Boolean(consumeOption('serve', false));
    const doNoServe = Boolean(consumeOption('no-server', false));
    const langPattern = options.pop();
    validateOptions();

    const gcStressEnabled = await testFeatureFlag('DEBUG_STRESS_GC'); // Disable stress GC for benchmarks
    if (gcStressEnabled) {
      abort(
        'GC stress mode is enabled. Must be disabled for benchmarks to ensure consistent results.',
      );
    }

    if (doOnlyServe && doNoServe) {
      abort('Cannot specify both serve and no-serve options');
    }

    if (doOnlyServe) {
      info('Serving results');
      await serveResults();
      break;
    }

    await buildSlangConfig(BUILD_CONFIG_RELEASE_PROFILED);
    await runBenchmarks(langPattern);

    if (!doNoServe) {
      info('Serving results');
      await serveResults();
    }
    break;
  }
  case 'test': {
    const config = BUILD_CONFIG_RELEASE;
    const doUpdateFiles = Boolean(consumeOption('update-files', false));
    const testNamePattern = options.pop() || '.*';
    validateOptions();

    await checkFeatureFlagsForTests();
    await buildSlangConfig(config);
    await runTests(config, undefined, doUpdateFiles, testNamePattern);
    break;
  }
  case 'sample': {
    const config = BUILD_CONFIG_RELEASE;
    validateOptions();

    await buildSlangConfig(config);
    console.clear();
    info('Running slang file', SLANG_SAMPLE_FILE);
    const { stdoutOutput, stderrOutput } = await runSlangFile(SLANG_SAMPLE_FILE, config);

    separator();
    console.log(stdoutOutput);
    if (stderrOutput) {
      console.log(stderrOutput);
    }
    separator();
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
      async (signal, triggerFile, isFirstRun) => {
        // Only build if the trigger file is not the sample file, or if it is the first run
        if (isFirstRun || !sampleFilePath.endsWith(triggerFile)) {
          const didBuild = await buildSlangConfig(config, signal, false /* don't abort on error */);
          if (!didBuild) {
            return;
          }
        }

        info('Running slang file', sampleFilePath);
        const { stdoutOutput, stderrOutput } = await runSlangFile(
          sampleFilePath,
          config,
          signal,
          true,
        );
        console.clear();

        separator();
        console.log(stdoutOutput);
        if (stderrOutput) {
          console.log(stderrOutput);
        }
        separator();
      },
    );
    break;
  }
  case 'watch-test': {
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
      async (signal, triggerFile, isFirstRun) => {
        // Only build if the trigger file is not a test file, or if it is the first run
        if (isFirstRun || !triggerFile.endsWith(SLANG_TEST_SUFFIX)) {
          await checkFeatureFlagsForTests();
          const didBuild = await buildSlangConfig(config, signal, false /* don't abort on error */);
          if (!didBuild) {
            return;
          }
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
