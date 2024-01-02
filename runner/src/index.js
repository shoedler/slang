import { runBenchmarks } from "./bench.js";
import {
  BUILD_CONFIG_DEBUG,
  BUILD_CONFIG_RELEASE,
  SLANG_PROJ_DIR,
  SLANG_SAMPLE_FILE,
  SLANG_TEST_SUFFIX,
} from "./config.js";
import { runTests } from "./test.js";
import { buildSlangConfig, exitWithError, info, runSlangFile, separator, warn } from "./utils.js";
import { watch } from "./watch.js";

const cmd = process.argv[2];

switch (cmd) {
  case "bench": {
    const configs = [BUILD_CONFIG_DEBUG, BUILD_CONFIG_RELEASE];
    for (const config of configs) {
      await buildSlangConfig(config);
    }
    await runBenchmarks(configs);
    break;
  }
  case "test": {
    const config = BUILD_CONFIG_RELEASE;
    await buildSlangConfig(config);
    await runTests(config);
    break;
  }
  case "watch-file": {
    const config = BUILD_CONFIG_RELEASE;
    const filePath = SLANG_SAMPLE_FILE;
    watch(
      SLANG_PROJ_DIR,
      { recursive: true },
      (filename) =>
        filename.endsWith(".c") || filename.endsWith(".h") || filePath.endsWith(filename),
      async (signal) => {
        console.clear();
        await buildSlangConfig(config, signal);
        info("Running slang file", filePath);
        info("Stdout and stderr might not be in order");
        const { exitCode, rawOutput } = await runSlangFile(filePath, config, signal, true);

        separator();
        console.log(rawOutput);
        separator();

        if (exitCode === 0) {
          info("Ran with 0 exit code");
        } else {
          warn("Ran with non-zero exit code", exitCode);
        }
      }
    );
    break;
  }
  case "watch-tests": {
    const config = BUILD_CONFIG_RELEASE;
    watch(
      SLANG_PROJ_DIR,
      { recursive: true },
      (filename) =>
        filename.endsWith(".c") || filename.endsWith(".h") || filename.endsWith(SLANG_TEST_SUFFIX),
      async (signal) => {
        await buildSlangConfig(config, signal);
        await runTests(config, signal);
      }
    );
    break;
  }
  default: {
    exitWithError("Unknown command", `Command: ${cmd}`);
  }
}
