import chalk from 'chalk';
import path from 'node:path';
import os from 'node:os';
import { Worker } from 'node:worker_threads';
import { SLANG_RUNNER_SRC_DIR, SLANG_TEST_DIR, SLANG_TEST_SUFFIX } from './config.js';
import {
  LOG_CONFIG,
  abort,
  extractCommentMetadata,
  fail,
  findFiles,
  info,
  pass,
  runSlangFile,
  skip,
  ok,
  updateCommentMetadata,
} from './utils.js';

const SKIP = 'Skip';
const EXIT = 'Exit';
const EXPECT = 'Expect';
const EXPECT_ERROR = 'ExpectError';

export const WORK_TYPE_RUN_TEST = 'runTest';
export const WORKER_MESSAGE_TYPE_RESULT = 'result';

/**
 * Represents a test result
 * @typedef {{
 *   testFilepath: string,
 *   passed: boolean,
 *   skipped: boolean,
 *   skipReason?: string,
 *   errorMessages: string[],
 *   assertions: number
 * }} TestResult
 */

const DEFAULT_TEST_NAME_PATTERN = '.*';
/**
 * Finds all tests in the slang bench directory
 * @param {string} testNamePattern - A pattern to filter tests by name. Supports regex, defaults to all tests '.*'
 * @returns {Promise<string[]>} - Array of test file paths
 */
export const findTests = async (testNamePattern = DEFAULT_TEST_NAME_PATTERN) => {
  const findAllTests = testNamePattern === DEFAULT_TEST_NAME_PATTERN;
  const regex = new RegExp(testNamePattern);
  const tests = (await findFiles(SLANG_TEST_DIR, SLANG_TEST_SUFFIX)).filter(path => regex.test(path));

  if (tests.length === 0) {
    abort(`No tests found in ${SLANG_TEST_DIR} with suffix ${SLANG_TEST_SUFFIX} matching pattern ${testNamePattern}`);
  }

  if (findAllTests) {
    info(`All Tests. Found ${tests.length} slang tests`);
  } else {
    const allTestsString = tests.join('\n');
    info(
      `Found ${tests.length} slang tests matching pattern /${testNamePattern}/ in ${SLANG_TEST_DIR}`,
      '\n' + allTestsString,
    );
  }

  return tests;
};

/**
 * Creates a CLI progress display for parallel test execution
 * @returns {{
 *   prepareDisplay: () => void,
 *   updateWorker: (workerId: number, status: string) => void,
 *   closeDisplay: () => void
 * }}
 */
const createProgressDisplay = numWorkers => {
  const workerLines = new Map();
  const [infoHeaderPrefix, infoStyleFn] = LOG_CONFIG['info'];
  const infoHeader = infoStyleFn(infoHeaderPrefix);

  let startLine = 0;

  const prepareDisplay = () => {
    // Save current cursor position
    process.stdout.write('\x1b7');

    // Add numWorkers empty lines
    for (let i = 0; i < numWorkers; i++) {
      process.stdout.write('\n');
    }

    // Save the starting line number for our display
    startLine = process.stdout.rows - numWorkers;

    process.stdout.write('\x1b8'); // Restore cursor to previous position
    process.stdout.write('\x1b[?25l'); // Hide cursor
  };

  const updateWorker = (workerId, status) => {
    if (!workerLines.has(workerId)) {
      workerLines.set(workerId, startLine + workerLines.size);
    }

    const line = workerLines.get(workerId);

    // Save cursor position
    process.stdout.write('\x1b7');

    // Move cursor to worker's line and clear it
    process.stdout.write(`\x1b[${line};1H\x1b[2K`);
    process.stdout.write(infoHeader);
    process.stdout.write(` Runner ${workerId.toString().padEnd(2, ' ')} - ${status}`);

    // Restore cursor position
    process.stdout.write('\x1b8');
  };

  const closeDisplay = () => {
    process.stdout.write('\x1b[?25h'); // Show cursor
  };

  process.on('exit', () => {
    process.stdout.write('\x1b[?25h'); // Show cursor
  });

  return { prepareDisplay, updateWorker, closeDisplay };
};

/**
 * Helper function to compare the output of a test to a set of expectations. Also provides a set of updated metadata for expectations that failed.
 * @param {string} rawTestOutput - The raw output of the test (stdout or stderr)
 * @param {Object[]} metadata - The expectations to compare the output to.
 * @param {string} expectationType - The type of the expectations (Expect or ExpectError). Just used as a sanity check for the metadata. (All metadata should be of the same type)
 * @returns {{
 *   failedAssertions: Array<{actual: string, expected: string, line: number}>,
 *   unhandledAssertions: Array<{type: string, value: string, line: number}>,
 *   unhandledOutput: string[],
 *   updatedMetadata: Array<{type: string, value: string, line: number}>,
 *   assertions: number
 * }} - The comparison results.
 */
const makeComparison = (rawTestOutput, metadata, expectationType) => {
  const lines = rawTestOutput
    .split('\r\n')
    .filter(Boolean)
    .map(line => line.trimEnd());

  const failedAssertions = [];
  const unhandledAssertions = [];
  const unhandledOutput = [];
  const updatedMetadata = []; // This is used to update the test file e.g. after changing error messages.

  // Only compare up to minLen - excess output or expectations are handled later.
  let minLen = Math.min(metadata.length, lines.length);

  for (let i = 0; i < minLen; i++) {
    const { type, value: expected, line } = metadata[i];

    if (type !== expectationType) {
      abort(`Sanity check failed. Invalid expectation type ${type} for comparison. Expected ${expectationType}`);
    }

    const actual = lines[i];

    if (actual !== expected) {
      failedAssertions.push({ actual, expected, line });
      updatedMetadata.push({ type, value: actual, line });
    }
  }

  // Handle excess output or expectations
  for (let i = minLen; i < metadata.length; i++) {
    unhandledAssertions.push(metadata[i]);
  }

  for (let i = minLen; i < lines.length; i++) {
    unhandledOutput.push(lines[i]);
  }

  return {
    failedAssertions,
    unhandledAssertions,
    unhandledOutput,
    updatedMetadata,
    assertions: minLen,
  };
};

/**
 * Formats and prints the test results summary
 * @param {TestResult[]} results
 */
const printSummary = results => {
  const totalTests = results.length;
  const passedTests = results.filter(r => r.passed).length;
  const skippedTests = results.filter(r => r.skipped).length;
  const totalAssertions = results.reduce((sum, r) => sum + r.assertions, 0);

  ok('Done running tests. ');
  const summaryMessage = `Summary: ${passedTests}/${totalTests} passed, ${skippedTests} skipped, ${totalAssertions} assertions`;

  if (passedTests === totalTests - skippedTests) {
    info(chalk.green(summaryMessage));
    return;
  }

  info(chalk.red(summaryMessage));
  info(chalk.red('Failed tests:'));

  results
    .filter(r => !r.passed && !r.skipped)
    .forEach(({ testFilepath, errorMessages }) => {
      console.log(chalk.bgWhite.black(` ${testFilepath} `));
      console.log(errorMessages.join('\n') + '\n');
    });
};

/**
 * Helper function to evaluate a comparison.
 * @param {{
 *   failedAssertions: { actual: string, expected: string, line: number }[],
 *   unhandledAssertions: { type: string, value: string, line: number }[],
 *   unhandledOutput: string[],
 *   updatedMetadata: { type: string, value: string, line: number }[],
 *   assertions: number,
 * }} comparison - The comparison results.
 * @param {string} expectationType - The type of the expectations (Expect or ExpectError).
 * @param {'stdout'|'stderr'} expectedStream - The expected stream type. (stdout or stderr)
 * @returns {string[]} - An array of error messages.
 */
const createErrorMessages = (comparison, expectationType, expectedStream) => {
  const errorMessages = [];

  if (comparison.failedAssertions.length > 0) {
    errorMessages.push(chalk.bold(`▬ Test has failed [${expectationType}]-assertions:`));
    for (const { actual, expected, line } of comparison.failedAssertions) {
      errorMessages.push(
        `${chalk.red(` × ${expectationType}:`)} ${expected}\n` +
          `${chalk.red('   Actual:')} ${actual} ${chalk.blue('Tagged on line:')} ${line}`,
      );
    }
  }

  if (comparison.unhandledAssertions.length > 0) {
    errorMessages.push(chalk.bold(`▬ Test specifies more [${expectationType}]-expectations than output:`));
    for (const { value: expected, line } of comparison.unhandledAssertions) {
      errorMessages.push(
        `${chalk.red(` × Unsatisfied assertion. ${expectationType}: `)} ${expected} ${chalk.blue(
          'Tagged on line:',
        )} ${line}`,
      );
    }
  }

  if (comparison.unhandledOutput.length > 0) {
    errorMessages.push(chalk.bold(`▬ Execution generated more ${expectedStream}-output than expected:`));
    for (const actual of comparison.unhandledOutput) {
      errorMessages.push(`${chalk.red(` × Unhandled output. (in ${expectedStream}): `)} ${actual}`);
    }
  }

  return errorMessages;
};

/**
 * Runs a single test and returns the result
 * @param {string} testFilepath - Test filepath
 * @param {string} buildConfig - Build configuration
 * @param {boolean} doUpdateFile - Whether to update test file with new expectations from current run - if possible
 * @param {AbortSignal} signal - Abort signal
 * @returns {Promise<TestResult>}
 */
export const runSingleTest = async (testFilepath, buildConfig, doUpdateFile, signal) => {
  const commentMetadata = await extractCommentMetadata(testFilepath);
  const skipMetadata = commentMetadata.find(m => m.type === SKIP);

  if (skipMetadata) {
    return {
      testFilepath,
      passed: false,
      skipped: true,
      skipReason: skipMetadata.value,
      errorMessages: [],
      assertions: 0,
    };
  }

  // Setup
  const expectationsMetadata = commentMetadata.filter(m => m.type === EXPECT);
  const errorExpectationsMetadata = commentMetadata.filter(m => m.type === EXPECT_ERROR);
  const exitMetadata = commentMetadata.find(m => m.type === EXIT);

  // Execute the test
  const { stdoutOutput, stderrOutput, exitCode } = await runSlangFile(testFilepath, buildConfig, signal);
  const errorMessages = [];

  // Check exit code
  const expectedExitCode = parseInt(exitMetadata?.value ?? '0', 10);
  if (exitCode !== 0 && exitCode !== expectedExitCode) {
    errorMessages.push(chalk.bold(`▬ Test exited with unexpected non-zero exit code ${chalk.bgRed(exitCode)}.`));
  } else if (exitCode === 0 && expectedExitCode !== 0) {
    errorMessages.push(chalk.bold('▬ Test successfully exited, but was expected to fail.'));
  }

  // Compare outputs
  const comparison = makeComparison(stdoutOutput, expectationsMetadata, EXPECT);
  const errorComparison = makeComparison(stderrOutput, errorExpectationsMetadata, EXPECT_ERROR);

  // Update stats and add error messages
  const totalAssertions = comparison.assertions + errorComparison.assertions;
  errorMessages.push(...createErrorMessages(comparison, EXPECT, 'stdout'));
  errorMessages.push(...createErrorMessages(errorComparison, EXPECT_ERROR, 'stderr'));

  // If specified, update the test file with new expectations if possible
  if (doUpdateFile && errorMessages.length !== 0) {
    // We can only update if the comparison has no unsatisfied expectations
    if (comparison.unhandledAssertions.length === 0) {
      await updateCommentMetadata(testFilepath, comparison.updatedMetadata, EXPECT, comparison.unhandledOutput);
    }
    if (errorComparison.unhandledAssertions.length === 0) {
      await updateCommentMetadata(
        testFilepath,
        errorComparison.updatedMetadata,
        EXPECT_ERROR,
        errorComparison.unhandledOutput,
      );
    }
  }

  return {
    testFilepath,
    passed: errorMessages.length === 0,
    skipped: false,
    errorMessages,
    assertions: totalAssertions,
  };
};

/**
 * Runs all test either sequentially or in parallel
 * @param {string} buildConfig - Build configuration to use
 * @param {AbortSignal} signal - Abort signal to use
 * @param {boolean} doUpdateFiles - Whether to update test files with new expectations from current run - if possible
 * @param {string} testFilepaths - An array of absolute file paths to test files to run
 * @param {boolean} parallel - Whether to run tests in parallel
 */
export const runTests = async (buildConfig, signal, doUpdateFiles = false, testFilepaths, parallel = true) => {
  if (!parallel) {
    const results = [];
    for (const testFilepath of testFilepaths) {
      const [header, headerStyle] = LOG_CONFIG['info'];
      process.stdout.write(headerStyle(header));
      process.stdout.write(`  Running ${chalk.bold(path.basename(testFilepath))}`);
      const result = await runSingleTest(testFilepath, buildConfig, doUpdateFiles, signal);
      process.stdout.write('\r' + ' '.repeat(header.length + 2) + '\r'); // Clear the line

      results.push(result);

      if (result.skipped) {
        skip(path.basename(testFilepath) + chalk.gray(` Filepath: ${testFilepath}, Reason: ${result.skipReason}`));
      } else if (result.passed) {
        pass(path.basename(testFilepath) + chalk.gray(` Filepath: ${testFilepath}`));
      } else {
        fail(
          path.basename(testFilepath) +
            chalk.gray(` Filepath: ${testFilepath}`) +
            `\n${result.errorMessages.join('\n')}`,
        );
      }
    }
    printSummary(results);
    return;
  }

  // Parallel execution
  const cpus = os.cpus().length;
  const numWorkers = cpus < testFilepaths.length ? cpus : testFilepaths.length;
  const display = createProgressDisplay(numWorkers);
  let currentTestIndex = 0;

  info(`Initializing ${numWorkers} test runners for parallel test execution...`);

  /** @type {TestResult[]} */
  const results = [];
  let displayPrepared = false;

  display.prepareDisplay();

  const [failHeaderPrefix, failStyleFn] = LOG_CONFIG['fail'];
  const [skipHeaderPrefix, skipStyleFn] = LOG_CONFIG['skip'];
  const [passHeaderPrefix, passStyleFn] = LOG_CONFIG['pass'];
  const failHeader = failStyleFn(failHeaderPrefix);
  const skipHeader = skipStyleFn(skipHeaderPrefix);
  const passHeader = passStyleFn(passHeaderPrefix);

  const workerPool = Array.from({ length: numWorkers }, (_, i) => {
    const worker = new Worker(path.join(SLANG_RUNNER_SRC_DIR, 'test-worker.js'));

    worker.on('online', () => {
      if (!displayPrepared) {
        displayPrepared = true;
      }
      display.updateWorker(i, 'Initializing...');
    });

    worker.on('message', ({ type, data }) => {
      if (type === WORKER_MESSAGE_TYPE_RESULT) {
        results.push(data);
        const testName = path.basename(data.testFilepath);
        if (data.skipped) {
          display.updateWorker(i, `${skipHeader} ${testName} (${data.skipReason})`);
        } else if (data.passed) {
          display.updateWorker(i, `${passHeader} ${testName}`);
        } else {
          display.updateWorker(i, `${failHeader} ${testName}`);
        }

        // Assign next test if available
        if (currentTestIndex < testFilepaths.length) {
          worker.postMessage({
            type: WORK_TYPE_RUN_TEST,
            test: testFilepaths[currentTestIndex++],
            config: buildConfig,
          });
        } else {
          worker.terminate();
          display.updateWorker(i, chalk.italic('done'));
        }
      }
    });

    // Assign initial test if available
    if (currentTestIndex < testFilepaths.length) {
      worker.postMessage({
        type: WORK_TYPE_RUN_TEST,
        test: testFilepaths[currentTestIndex++],
        config: buildConfig,
      });
    }

    return worker;
  });

  // Wait for all workers to complete
  await Promise.all(workerPool.map(worker => new Promise(resolve => worker.on('exit', resolve))));

  display.closeDisplay();

  printSummary(results);
};
