import chalk from 'chalk';
import path from 'node:path';
import { SLANG_TEST_DIR, SLANG_TEST_SUFFIX } from './config.js';
import {
  abort,
  extractCommentMetadata,
  fail,
  findFiles,
  info,
  pass,
  runSlangFile,
  updateCommentMetadata,
} from './utils.js';

const EXPECT = 'Expect';
const EXPECT_COMPILE_ERROR = 'ExpectCompileError';
const EXPECT_RUNTIME_ERROR = 'ExpectRuntimeError';

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
 * Runs all test
 * @param {string} config - Build configuration to use
 * @param {AbortSignal} signal - Abort signal to use
 * @param {boolean} updateFiles - Whether to update test files with new expectations
 */
export const runTests = async (config, signal, updateFiles = false) => {
  const tests = await findTests();
  const getTestName = filePath => path.parse(filePath).name;
  const failedTests = [];
  let totalAssertions = 0;

  info(`Running ${tests.length} slang tests`);

  for (const test of tests) {
    const commentMetadata = await extractCommentMetadata(test);
    const { output, exitCode } = await runSlangFile(test, config, signal);

    if (signal?.aborted) {
      throw signal.reason;
    }

    const minLen = Math.min(commentMetadata.length, output.length);
    const diffs = [];
    const metadataWithActual = []; // This is used to update the test file e.g. after changing error messages.
    // Just makes it easier to update the file

    // Check that expectations are met. Only check minLen - excess output or expectations are
    // handled in the evaluation
    for (let i = 0; i < minLen; i++) {
      const { type, value: expected, line } = commentMetadata[i];

      if (![EXPECT, EXPECT_COMPILE_ERROR, EXPECT_RUNTIME_ERROR].includes(type)) {
        abort('Unknown expectation type', `Type: ${type}`);
      }

      const actual = output[i];
      totalAssertions++;
      if (actual !== expected) {
        diffs.push({ actual, expected, line });
        metadataWithActual.push({ type, value: actual, line });
      }
    }

    const expectedCompileError = commentMetadata.some(m => m.type === EXPECT_COMPILE_ERROR);
    const expectedRuntimeError = commentMetadata.some(m => m.type === EXPECT_RUNTIME_ERROR);

    // Evaluate test results
    const errorMessages = [];
    if (exitCode === 65 && !expectedCompileError) {
      errorMessages.push(chalk.red('Test failed to compile but was not expected to.'));
    } else if (exitCode === 70 && !expectedRuntimeError) {
      errorMessages.push(chalk.red('Test encountered an unexpected runtime error.'));
    } else if (exitCode !== 0 && !expectedCompileError && !expectedRuntimeError) {
      errorMessages.push(chalk.red(`Test exited with unexpected non-zero exit code ${exitCode}.`));
    }

    // Handle diffs
    if (diffs.length > 0) {
      errorMessages.push(chalk.red('Test has failed assertions:'));
      for (const { actual, expected, line } of diffs) {
        errorMessages.push(
          `${chalk.red(' × Failed assertion. Expected:')} ${expected} ${chalk.red(
            'Actual:',
          )} ${actual} ${chalk.blue('Tagged on line:')} ${line}`,
        );
      }
    }

    // Handle excess output or expectations
    if (commentMetadata.length > minLen) {
      errorMessages.push(chalk.red('Test has more expectations than output:'));
      for (let i = minLen; i < commentMetadata.length; i++) {
        const { _, value: expected, line } = commentMetadata[i];
        errorMessages.push(
          `${chalk.red(
            ' × Unsatisfied assertion. Expected',
            chalk.italic(`(expectation index ${i}):`),
          )} ${expected} ${chalk.blue('Tagged on line:')} ${line}`,
        );
      }
    } else if (output.length > minLen) {
      errorMessages.push(chalk.red('Test has more output than expected:'));
      for (let i = minLen; i < output.length; i++) {
        const actual = output[i];
        errorMessages.push(
          `${chalk.red(
            ' × Unhandled output. Actual',
            chalk.italic(`(output index ${i}):`),
          )} ${actual}`,
        );
      }
    }

    if (updateFiles) {
      // If the test only had failed assertions, we update the test file.
      // This is only temporary and will be removed once the files are updated.
      if (diffs.length > 0 && diffs.length + 1 === errorMessages.length) {
        updateCommentMetadata(test, metadataWithActual);
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
    fail(
      `${chalk.red(finishMessage)} ${chalk.red('Failed tests:')}\n${failedTests
        .map(
          ({ test, errorMessages }) =>
            `${chalk.red(' █ ')}${getTestName(test)}\n${chalk.red(' │ ')}${errorMessages.join(
              `\n${chalk.red(' │ ')}`,
            )}`,
        )
        .join('\n')}`,
    );
  }
};
