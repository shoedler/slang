import chalk from 'chalk';
import path from 'node:path';
import { SLANG_TEST_DIR, SLANG_TEST_SUFFIX } from './config.js';
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
  updateCommentMetadata,
} from './utils.js';

const SKIP = 'Skip';
const EXIT = 'Exit';
const EXPECT = 'Expect';
const EXPECT_ERROR = 'ExpectError';

/**
 * Finds all tests in the slang bench directory
 * @returns {string[]} - Array of test file paths
 */
const findTests = async () => {
  const tests = await findFiles(SLANG_TEST_DIR, SLANG_TEST_SUFFIX);

  if (tests.length === 0)
    abort(`No tests found in ${SLANG_TEST_DIR} with suffix ${SLANG_TEST_SUFFIX}`);

  info(`Found ${tests.length} slang tests`);

  return tests;
};

/**
 * Helper function to compare the output of a test to a set of expectations. Also provides a set of updated metadata for expectations that failed.
 * @param {string} rawTestOutput - The raw output of the test (stdout or stderr)
 * @param {Object[]} metadata - The expectations to compare the output to.
 * @param {string} expectationType - The type of the expectations (Expect or ExpectError). Just used as a sanity check for the metadata. (All metadata should be of the same type)
 * @returns {{
 *   failedAssertions: { actual: string, expected: string, line: number }[],
 *   unhandledAssertions: { type: string, value: string, line: number }[],
 *   unhandledOutput: string[],
 *   updatedMetadata: { type: string, value: string, line: number }[],
 *   assertions: number,
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
      abort(
        `Sanity check failed. Invalid expectation type ${type} for comparison. Expected ${expectationType}`,
      );
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
 * Helper function to evaluate a comparison.
 * @param {{
 *   failedAssertions: { actual: string, expected: string, line: number }[],
 *   unhandledAssertions: { type: string, value: string, line: number }[],
 *   unhandledOutput: string[],
 *   updatedMetadata: { type: string, value: string, line: number }[],
 *   assertions: number,
 * }} comparison - The comparison results.
 * @param {string} expectationType - The type of the expectations (Expect or ExpectError).
 * @param {string} expectedStream - The expected stream type. (stdout or stderr)
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
    errorMessages.push(
      chalk.bold(`▬ Test specifies more [${expectationType}]-expectations than output:`),
    );
    for (const { value: expected, line } of comparison.unhandledAssertions) {
      errorMessages.push(
        `${chalk.red(` × Unsatisfied assertion. ${expectationType}: `)} ${expected} ${chalk.blue(
          'Tagged on line:',
        )} ${line}`,
      );
    }
  }

  if (comparison.unhandledOutput.length > 0) {
    errorMessages.push(
      chalk.bold(`▬ Execution generated more ${expectedStream}-output than expected:`),
    );
    for (const actual of comparison.unhandledOutput) {
      errorMessages.push(`${chalk.red(` × Unhandled output. (in ${expectedStream}): `)} ${actual}`);
    }
  }

  return errorMessages;
};

/**
 * Runs all test
 * @param {string} config - Build configuration to use
 * @param {AbortSignal} signal - Abort signal to use
 * @param {boolean} updateFiles - Whether to update test files with new expectations
 * @param {string} testNamePattern - A pattern to filter tests by name. Supports regex
 */
export const runTests = async (config, signal, updateFiles = false, testNamePattern = '.*') => {
  const tests = (await findTests()).filter(test => new RegExp(testNamePattern).test(test));
  const getTestName = filePath => path.parse(filePath).name;
  const failedTests = [];
  let totalAssertions = 0;

  info(`Running ${tests.length} slang tests`);

  for (const test of tests) {
    // Extract metadata from the test file and split into different types
    const commentMetadata = await extractCommentMetadata(test);
    const expectationsMetadata = commentMetadata.filter(m => m.type === EXPECT);
    const errorExpectationsMetadata = commentMetadata.filter(m => m.type === EXPECT_ERROR);
    const skipMetadata = commentMetadata.find(m => m.type === SKIP);
    const exitMetadata = commentMetadata.find(m => m.type === EXIT);

    // Exit early if the test is marked as skipped
    if (skipMetadata) {
      skip(getTestName(test) + chalk.gray(` Filepath: ${test}, Reason: ${skipMetadata.value}`));
      continue;
    }

    // Run the test
    const [header, headerStyle] = LOG_CONFIG['info'];
    process.stdout.write(headerStyle(header));
    process.stdout.write(`  Running ${chalk.bold(getTestName(test))}`);
    const { stdoutOutput, stderrOutput, exitCode } = await runSlangFile(test, config, signal);
    process.stdout.write('\r' + ' '.repeat(header.length + 2) + '\r'); // Clear the line

    // If the signal triggered and caused the test to exit early, we throw to stop the rest of the tests
    if (signal?.aborted) {
      throw signal.reason;
    }

    // Collect all error messages, so we can provide a summary at the end
    const errorMessages = [];

    // Before we start comparing, check if the exit code is as expected
    const expectedExitCode = parseInt(exitMetadata?.value ?? '0', 10);
    if (exitCode !== 0 && exitCode !== expectedExitCode) {
      errorMessages.push(
        chalk.bold(`▬ Test exited with unexpected non-zero exit code ${chalk.bgRed(exitCode)}.`),
      );
    } else if (exitCode === 0 && expectedExitCode !== 0) {
      errorMessages.push(chalk.bold('▬ Test successfully exited, but was expected to fail.'));
    }

    // Compare stdout and stderr to expectations
    const comparison = makeComparison(stdoutOutput, expectationsMetadata, EXPECT);
    const errorComparison = makeComparison(stderrOutput, errorExpectationsMetadata, EXPECT_ERROR);

    totalAssertions += comparison.assertions + errorComparison.assertions;

    // Create error messages for stdout and stderr
    errorMessages.push(...createErrorMessages(comparison, EXPECT, 'stdout'));
    errorMessages.push(...createErrorMessages(errorComparison, EXPECT_ERROR, 'stderr'));

    if (updateFiles && errorMessages.length !== 0) {
      // We can only update if the comparison has no unsatisfied expectations
      if (comparison.unhandledAssertions.length === 0) {
        await updateCommentMetadata(
          test,
          comparison.updatedMetadata,
          EXPECT,
          comparison.unhandledOutput,
        );
      }
      if (errorComparison.unhandledAssertions.length === 0) {
        await updateCommentMetadata(
          test,
          errorComparison.updatedMetadata,
          EXPECT_ERROR,
          errorComparison.unhandledOutput,
        );
      }
    }

    // Print test results
    if (errorMessages.length > 0) {
      fail(getTestName(test) + chalk.gray(` Filepath: ${test}`) + `\n${errorMessages.join('\n')}`);
      failedTests.push({ test, errorMessages });
    } else {
      pass(getTestName(test) + chalk.gray(` Filepath: ${test}`));
    }
  }

  let passedAmount = tests.length - failedTests.length;
  const allPassed = passedAmount === tests.length;
  const finishMessage =
    'Summary. ' + `${passedAmount}/${tests.length} passed, made ${totalAssertions} assertions.`;

  if (allPassed) {
    pass(chalk.green(finishMessage));
  } else {
    // Don't bother with the summary if there are not many tests
    if (tests.length <= 20) {
      fail(chalk.red(finishMessage));
      return;
    }

    fail(
      chalk.red(finishMessage) +
        ' Failed tests:\n\n' +
        failedTests
          .map(
            ({ test, errorMessages }) =>
              `${chalk.bgWhite.black(' ' + test + ' ')}\n${errorMessages.join('\n')}\n`,
          )
          .join('\n'),
    );
  }
};
