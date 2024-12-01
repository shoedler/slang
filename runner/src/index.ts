import process from 'node:process';
import { runBenchmarks, serveResults } from './bench.ts';
import {
  SLANG_PROJ_DIR,
  SlangBuildConfigs,
  SlangDefines,
  SlangFileSuffixes,
  SlangPaths,
  SlangRunFlags,
} from './config.ts';
import { findTests, runTests } from './test.ts';
import { abort, buildSlangConfig, info, runSlangFile, separator, testFeatureFlag, warn } from './utils.ts';
import { watch } from './watch.ts';

// Process arguments. This is a simple command line interface for running benchmarks and tests
// process.argv contains
// - 0: node executable path
// - 1: script path
// - 2: command
// - 3: options
// - 4: options
// - ...
const cmd = process.argv[2];
const options = process.argv.slice(3);

// Consume (remove) an option from the options array an return it's value. If the option is not found, return the default value
const consumeOption = (opt: string, defaultValue: boolean) => {
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
  '  - bench           Run benchmarks (debug & release) and serve results',
  '    - serve         Only serve benchmark results',
  '    - no-serve      Run benchmarks without serving results',
  '    - no-build      Skip building the project (default is to build)',
  '    - <pattern>     Run language that matches the regex pattern',
  '  - sample          Run sample file (sample.sl)',
  '  - test            Run tests (.spec.sl files)',
  '    - update-files  Update test files with new expectations',
  '    - no-parallel   Run tests sequentially (default is parallel)',
  '    - no-build      Skip building the project (default is to build)',
  '    - <pattern>     Run tests that match the regex pattern',
  '  - watch-sample    Watch sample file (sample.sl)',
  '  - watch-test      Watch test files',
  '    - no-parallel   Run tests sequentially (default is parallel)',
  '    - <pattern>     Watch tests that match the regex pattern',
  '',
  'Note: If not specified, the default configuration is release',
];

switch (cmd) {
  case 'bench': {
    const doOnlyServe = Boolean(consumeOption('serve', false));
    const doNoServe = Boolean(consumeOption('no-server', false));
    const doNoBuild = Boolean(consumeOption('no-build', false));

    const langPattern = options.pop();
    validateOptions();

    if (doOnlyServe && doNoServe) {
      abort('Cannot specify both serve and no-serve options');
    }

    if (doOnlyServe) {
      info('Serving results');
      await serveResults();
      break;
    }

    if (!doNoBuild) {
      await buildSlangConfig(SlangBuildConfigs.ReleaseProfiled);
    } else {
      warn('Skipping build');
    }

    await runBenchmarks(langPattern);

    if (!doNoServe) {
      info('Serving results');
      await serveResults();
    }
    break;
  }
  case 'test': {
    const config = SlangBuildConfigs.Release;
    const doUpdateFiles = Boolean(consumeOption('update-files', false));
    const doNoParallel = Boolean(consumeOption('no-parallel', false));
    const doNoBuild = Boolean(consumeOption('no-build', false));
    const testNamePattern = options.pop() || '.*';
    validateOptions();

    const ansiColorsEnabled = await testFeatureFlag(SlangDefines.EnableColorOutput);
    if (ansiColorsEnabled) {
      abort('ANSI colors are enabled. Must be disabled for tests to ensure consistent results.');
    }

    const testFilepaths = await findTests(testNamePattern);
    if (!doNoBuild) {
      await buildSlangConfig(config);
    } else {
      warn('Skipping build');
    }

    await runTests(config, testFilepaths, [SlangRunFlags.StressGc], null, doUpdateFiles, !doNoParallel);
    break;
  }
  case 'sample': {
    const config = SlangBuildConfigs.Release;
    validateOptions();

    await buildSlangConfig(config, null, true, `EXTRA_CFLAGS="-D${SlangDefines.EnableColorOutput}"`);
    console.clear();
    info('Running slang file', SlangPaths.SampleFile);
    const { stdoutOutput, stderrOutput } = await runSlangFile(SlangPaths.SampleFile, config);

    separator();
    console.log(stdoutOutput);
    if (stderrOutput) {
      console.log(stderrOutput);
    }
    separator();
    break;
  }
  case 'watch-sample': {
    const config = SlangBuildConfigs.Release;
    const sampleFilePath = SlangPaths.SampleFile;
    validateOptions();

    watch(
      SLANG_PROJ_DIR,
      { recursive: true },
      (filename: string) => filename.endsWith('.c') || filename.endsWith('.h') || sampleFilePath.endsWith(filename),
      async (signal, triggerFilename) => {
        // Only build if the trigger file is not the sample file, or if it is the first run
        if (!triggerFilename || !sampleFilePath.endsWith(triggerFilename)) {
          const didBuild = await buildSlangConfig(
            config,
            signal,
            false /* don't abort on error */,
            `EXTRA_CFLAGS="-D${SlangDefines.EnableColorOutput}"`,
          );
          if (!didBuild) {
            return;
          }
        }

        info('Running slang file', sampleFilePath);
        const { stdoutOutput, stderrOutput } = await runSlangFile(sampleFilePath, config, [], signal);
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
    const config = SlangBuildConfigs.Release;
    const doNoParallel = Boolean(consumeOption('no-parallel', false));
    const testNamePattern = options.pop() || '.*';
    validateOptions();

    watch(
      SLANG_PROJ_DIR,
      { recursive: true },
      filename =>
        filename.endsWith('.c') ||
        filename.endsWith('.h') ||
        (filename.endsWith(SlangFileSuffixes.Test) && new RegExp(testNamePattern).test(filename)),
      async (signal, triggerFilename) => {
        // Only build if the trigger file is not a test file, or if it is the first run
        if (!triggerFilename || !triggerFilename.endsWith(SlangFileSuffixes.Test)) {
          const ansiColorsEnabled = await testFeatureFlag(SlangDefines.EnableColorOutput);
          if (ansiColorsEnabled) {
            abort('ANSI colors are enabled. Must be disabled for tests to ensure consistent results.');
          }
          const didBuild = await buildSlangConfig(config, signal, false /* don't abort on error */);
          if (!didBuild) {
            return;
          }
        }

        const testFilepaths = await findTests(testNamePattern);
        await runTests(config, testFilepaths, [], signal, false, !doNoParallel);
      },
    );
    break;
  }
  default: {
    abort('Unknown command', hint.join('\n'));
  }
}
