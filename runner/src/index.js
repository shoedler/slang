import { runBenchmarks } from './bench.js';
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

const cmd = process.argv[2];

switch (cmd) {
  case 'bench': {
    const configs = [BUILD_CONFIG_DEBUG, BUILD_CONFIG_RELEASE];
    for (const config of configs) {
      await buildSlangConfig(config);
    }
    await runBenchmarks(configs);
    break;
  }
  case 'test': {
    const config = BUILD_CONFIG_RELEASE;
    await buildSlangConfig(config);
    await runTests(config, undefined, process.argv[3] === 'update-files');
    break;
  }
  case 'watch-sample': {
    const config = BUILD_CONFIG_RELEASE;
    const sampleFilePath = SLANG_SAMPLE_FILE;
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
    watch(
      SLANG_PROJ_DIR,
      { recursive: true },
      filename =>
        filename.endsWith('.c') || filename.endsWith('.h') || filename.endsWith(SLANG_TEST_SUFFIX),
      async signal => {
        const didBuild = await buildSlangConfig(config, signal, false /* don't abort on error */);
        if (!didBuild) {
          return;
        }

        await runTests(config, signal);
      },
    );
    break;
  }
  default: {
    const hint = [
      'Available commands:',
      '  - bench           Run benchmarks',
      '  - test            Run tests (.spec.sl files)',
      '    - update-files  Update test files with new expectations',
      '  - watch-sample    Watch sample file',
      '  - watch-tests     Watch test files',
    ];

    abort('Unknown command', hint.join('\n'));
  }
}
