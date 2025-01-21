import chalk from 'chalk';
import { PathLike } from 'node:fs';
import fs from 'node:fs/promises';
import os from 'node:os';
import path from 'node:path';
import process from 'node:process';
import { Worker } from 'node:worker_threads';
import { SlangBuildConfigs, SlangFileSuffixes, SlangPaths, SlangRunFlags } from './config.ts';
import { abort, fail, findFiles, info, LOG_CONFIG, ok, pass, readFile, runSlangFile, skip, warn } from './utils.ts';

export enum WorkerMessageTypes {
  Result = 'result',
}
export type WorkerMessageArgs = {
  [WorkerMessageTypes.Result]: { result: TestResult };
};

export enum WorkMessageTypes {
  RunTest = 'runTest',
}
export type WorkMessageArgs = {
  [WorkMessageTypes.RunTest]: {
    test: string;
    config: SlangBuildConfigs;
    runFlags: SlangRunFlags[];
    updateFile: boolean;
  };
};

type TestResult = {
  testFilepath: string;
  passed: boolean;
  skipped: boolean;
  skipReason?: string;
  errorMessages: string[];
  assertions: number;
};

type Metadata = {
  type: MetadataType;
  line: number;
  value: string;
};

enum MetadataType {
  Expect = 'expect',
  ExpectError = 'expect-error',
  Exit = 'exit',
  Skip = 'skip',
}

const metadataTypes = Object.keys(MetadataType).map(k => MetadataType[k as keyof typeof MetadataType]);
const isMetadataType = (type: string): type is MetadataType => metadataTypes.includes(type as MetadataType);

type FailedTestAssertion = {
  actual: string;
  expected: string;
  line: number;
};

type TestResultToMetadataComparison = {
  failedAssertions: FailedTestAssertion[];
  unhandledAssertions: Metadata[];
  unhandledOutput: string[];
  /** Updated metadata for updating a test file */
  updatedMetadata: Metadata[];
  assertions: number;
};

const DEFAULT_TEST_NAME_PATTERN = '.*';

/**
 * Finds all tests in the slang test directory
 * @param testNamePattern - A pattern to filter tests by name. Supports regex, defaults to all tests '.*'
 * @returns Array of test file paths
 */
export const findTests = async (testNamePattern: string = DEFAULT_TEST_NAME_PATTERN) => {
  const findAllTests = testNamePattern === DEFAULT_TEST_NAME_PATTERN;
  const regex = new RegExp(testNamePattern);
  const tests = (await findFiles(SlangPaths.TestDir, SlangFileSuffixes.Test)).filter(path => regex.test(path));

  if (tests.length === 0) {
    abort(
      `No tests found in ${SlangPaths.TestDir} with suffix ${SlangFileSuffixes.Test} matching pattern ${testNamePattern}`,
    );
  }

  if (findAllTests) {
    info(`All Tests. Found ${tests.length} slang tests`);
  } else {
    const allTestsString = tests.join('\n');
    info(
      `Found ${tests.length} slang tests matching pattern /${testNamePattern}/ in ${SlangPaths.TestDir}`,
      '\n' + allTestsString,
    );
  }

  return tests;
};

/**
 * Creates a CLI progress display for parallel test execution
 * @returns Object with functions to prepare, update and close the display
 */
const createProgressDisplay = (
  numWorkers: number,
): {
  prepareDisplay: () => void;
  updateWorker: (workerId: number, status: string) => void;
  closeDisplay: () => void;
} => {
  const workerLines = new Map();
  const [infoHeaderPrefix, infoStyleFn] = LOG_CONFIG['info'];
  const infoHeader = infoStyleFn(infoHeaderPrefix);

  let startLine = 0;

  const prepareDisplay = () => {
    process.stdout.write('\x1b7'); // Save current cursor position

    // Add numWorkers empty lines
    for (let i = 0; i < numWorkers; i++) {
      process.stdout.write('\n');
    }

    // Save the starting line number for our display
    startLine = process.stdout.rows - numWorkers;

    process.stdout.write('\x1b8'); // Restore cursor to previous position
    process.stdout.write('\x1b[?25l'); // Hide cursor
  };

  const updateWorker = (workerId: number, status: string) => {
    if (!workerLines.has(workerId)) {
      workerLines.set(workerId, startLine + workerLines.size);
    }

    const line = workerLines.get(workerId);

    process.stdout.write('\x1b7'); // Save cursor position

    // Move cursor to worker's line and clear it
    process.stdout.write(`\x1b[${line};1H\x1b[2K`);
    process.stdout.write(infoHeader);
    process.stdout.write(` Runner ${workerId.toString().padEnd(2, ' ')} - ${status}`);

    process.stdout.write('\x1b8'); // Restore cursor position
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
 * @param rawTestOutput - The raw output of the test (stdout or stderr)
 * @param metadata - The expectations to compare the output to.
 * @param expectationType - The type of the expectations (expect or expect-error). Just used as a sanity check for the metadata. (All metadata should be of the same type)
 * @returns The comparison results
 */
const makeComparison = (
  rawTestOutput: string,
  metadata: Metadata[],
  expectationType: MetadataType,
): TestResultToMetadataComparison => {
  const lines = rawTestOutput
    .split('\r\n')
    .filter(Boolean)
    .map(line => line.trimEnd());

  const failedAssertions: FailedTestAssertion[] = [];
  const unhandledAssertions: Metadata[] = [];
  const unhandledOutput: string[] = [];
  const updatedMetadata: Metadata[] = []; // This is used to update the test file e.g. after changing error messages.

  // Only compare up to minLen - excess output or expectations are handled later.
  const minLen = Math.min(metadata.length, lines.length);

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
 * @param results - The test results to summarize
 */
const printSummary = (results: TestResult[]) => {
  const totalTests = results.length;
  const passedTests = results.filter(r => r.passed).length;
  const skippedTests = results.filter(r => r.skipped).length;
  const failedTests = totalTests - passedTests - skippedTests;
  const totalAssertions = results.reduce((sum, r) => sum + r.assertions, 0);

  ok('Done running tests. ');
  const failedMessage = failedTests > 0 ? `, ${chalk.red(totalTests - passedTests - skippedTests)} failed` : '';
  const skippedMessage = skippedTests > 0 ? `, ${chalk.yellow(skippedTests)} skipped` : '';
  const assertionsMessage = `. Made ${chalk.magenta(totalAssertions)} assertions`;

  const summaryMessage =
    `Summary: ${chalk.green(passedTests)}/${chalk.bold(totalTests)} passed` +
    failedMessage +
    skippedMessage +
    assertionsMessage;

  info(summaryMessage);

  if (failedTests <= 0) {
    return;
  }

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
 * @param comparison - The comparison to evaluate.
 * @param expectationType - The type of the expectations (expect or expect-error).
 * @param expectedStream - The expected stream type. (stdout or stderr)
 * @returns An array of error messages.
 */
const createErrorMessages = (
  comparison: TestResultToMetadataComparison,
  expectationType: MetadataType,
  expectedStream: 'stderr' | 'stdout',
) => {
  const errorMessages = [];

  if (comparison.failedAssertions.length > 0) {
    errorMessages.push(chalk.bold(`▬ Test has failed [${expectationType}]-assertions:`));
    for (const { actual, expected, line } of comparison.failedAssertions) {
      errorMessages.push(
        `${chalk.red(` × ${expectationType}:`)} ${expected} (${chalk.blue('tagged on line:')} ${line})\n` +
          `${chalk.red('   actual:')} ${actual}`,
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
 * @param testFilepath - Test filepath
 * @param buildConfig - Build configuration
 * @param runFlags - Flags to pass to slang run
 * @param doUpdateFile - Whether to update test file with new expectations from current run - if possible
 * @param signal - Abort signal
 * @returns The result of the test
 */
export const runSingleTest = async (
  testFilepath: string,
  buildConfig: SlangBuildConfigs,
  runFlags: SlangRunFlags[] = [],
  doUpdateFile: boolean = false,
  signal: AbortSignal | null = null,
): Promise<TestResult> => {
  const commentMetadata = await extractCommentMetadata(testFilepath);
  const skipMetadata = commentMetadata.find(m => m.type === MetadataType.Skip);

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
  const expectationsMetadata = commentMetadata.filter(m => m.type === MetadataType.Expect);
  const errorExpectationsMetadata = commentMetadata.filter(m => m.type === MetadataType.ExpectError);
  const exitMetadata = commentMetadata.find(m => m.type === MetadataType.Exit);

  // Execute the test
  const { stdoutOutput, stderrOutput, exitCode } = await runSlangFile(testFilepath, buildConfig, runFlags, signal);
  const errorMessages = [];

  // Check exit code
  const expectedExitCode = parseInt(exitMetadata?.value ?? '0', 10);
  if (exitCode !== 0 && exitCode !== expectedExitCode) {
    errorMessages.push(chalk.bold(`▬ Test exited with unexpected non-zero exit code ${chalk.bgRed(exitCode)}.`));
  } else if (exitCode === 0 && expectedExitCode !== 0) {
    errorMessages.push(chalk.bold('▬ Test successfully exited, but was expected to fail.'));
  }

  // Compare outputs
  const comparison = makeComparison(stdoutOutput, expectationsMetadata, MetadataType.Expect);
  const errorComparison = makeComparison(stderrOutput, errorExpectationsMetadata, MetadataType.ExpectError);

  // Update stats and add error messages
  const totalAssertions = comparison.assertions + errorComparison.assertions;
  errorMessages.push(...createErrorMessages(comparison, MetadataType.Expect, 'stdout'));
  errorMessages.push(...createErrorMessages(errorComparison, MetadataType.ExpectError, 'stderr'));

  // If specified, update the test file with new expectations if possible
  if (doUpdateFile && errorMessages.length !== 0) {
    // We can only update if the comparison has no unsatisfied expectations
    if (comparison.unhandledAssertions.length === 0) {
      await updateCommentMetadata(
        testFilepath,
        comparison.updatedMetadata,
        MetadataType.Expect,
        comparison.unhandledOutput,
      );
    }
    if (errorComparison.unhandledAssertions.length === 0) {
      await updateCommentMetadata(
        testFilepath,
        errorComparison.updatedMetadata,
        MetadataType.ExpectError,
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
 * @param buildConfig - Build configuration to use
 * @param testFilepaths - An array of absolute file paths to test files to run
 * @param runFlags - Flags to pass to slang run
 * @param signal - Abort signal to use
 * @param doUpdateFiles - Whether to update test files with new expectations from current run - if possible
 * @param doParallel - Whether to run tests in parallel
 */
export const runTests = async (
  buildConfig: SlangBuildConfigs,
  testFilepaths: string[],
  runFlags: SlangRunFlags[] = [],
  signal: AbortSignal | null = null,
  doUpdateFiles = false,
  doParallel = true,
) => {
  if (!doParallel) {
    const results = [];
    for (const testFilepath of testFilepaths) {
      const [header, headerStyle] = LOG_CONFIG['info'];
      process.stdout.write(headerStyle(header));
      process.stdout.write(`  Running ${chalk.bold(path.basename(testFilepath))}`);
      const result = await runSingleTest(testFilepath, buildConfig, runFlags, doUpdateFiles, signal);
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

  const results: TestResult[] = [];
  let displayPrepared = false;

  display.prepareDisplay();

  const [failHeaderPrefix, failStyleFn] = LOG_CONFIG['fail'];
  const [skipHeaderPrefix, skipStyleFn] = LOG_CONFIG['skip'];
  const [passHeaderPrefix, passStyleFn] = LOG_CONFIG['pass'];
  const failHeader = failStyleFn(failHeaderPrefix);
  const skipHeader = skipStyleFn(skipHeaderPrefix);
  const passHeader = passStyleFn(passHeaderPrefix);

  const workerPool = Array.from({ length: numWorkers }, (_, i) => {
    const worker = new Worker(path.join(SlangPaths.RunnerSrcDir, 'test-worker.ts'));

    worker.on('online', () => {
      if (!displayPrepared) {
        displayPrepared = true;
      }
      display.updateWorker(i, 'Initializing...');
    });

    worker.on('message', arg => {
      const { type } = arg;
      if (type === WorkerMessageTypes.Result) {
        const { result } = <WorkerMessageArgs[WorkerMessageTypes.Result]>arg;
        results.push(result);

        const testName = path.basename(result.testFilepath);
        if (result.skipped) {
          display.updateWorker(i, `${skipHeader} ${testName} (${result.skipReason})`);
        } else if (result.passed) {
          display.updateWorker(i, `${passHeader} ${testName}`);
        } else {
          display.updateWorker(i, `${failHeader} ${testName}`);
        }

        // Assign next test if available
        if (currentTestIndex < testFilepaths.length) {
          worker.postMessage(<WorkMessageArgs[WorkMessageTypes.RunTest]>{
            type: WorkMessageTypes.RunTest,
            test: testFilepaths[currentTestIndex++],
            config: buildConfig,
            updateFile: doUpdateFiles,
            runFlags,
          });
        } else {
          worker.terminate();
          display.updateWorker(i, chalk.italic('done'));
        }
      }
    });

    // Assign initial test if available
    if (currentTestIndex < testFilepaths.length) {
      worker.postMessage(<WorkMessageArgs[WorkMessageTypes.RunTest]>{
        type: WorkMessageTypes.RunTest,
        test: testFilepaths[currentTestIndex++],
        config: buildConfig,
        updateFile: doUpdateFiles,
        runFlags,
      });
    }

    return worker;
  });

  // Wait for all workers to complete
  await Promise.all(workerPool.map(worker => new Promise(resolve => worker.on('exit', resolve))));

  display.closeDisplay();

  printSummary(results);
};

/**
 * Parses a slang file and returns comment-based metadata.
 * Metadata is contained in a line comment (//) and is surrounded by brackets.
 * Metadata can have a value which is everything to the right of the closing bracket.
 * @param file - Absolute path to a slang file
 * @returns Array containing metadata
 */
const extractCommentMetadata = async (file: PathLike): Promise<Metadata[]> => {
  const fileContents = await readFile(file);
  const lines = fileContents.split('\n');
  const metadata: Metadata[] = [];

  lines.forEach((l, i) => {
    const match = l.match(/\/\/\s*\[(.+?)\] (.*)/);
    if (match) {
      const [, type, value] = match;
      if (!isMetadataType(type)) {
        warn(`Unknown metadata type ${type} in ${file}. Ignoring...`);
      } else {
        metadata.push({ type, line: i + 1, value: value.trimEnd() });
      }
    }
  });
  return metadata;
};

/**
 * Update comment-based metadata in a slang file.
 * @param file - Absolute path to a slang file
 * @param metadata - Array containing the new metadata
 * @param expectationType - Type of metadata to update (e.g. 'expect', 'expect-error')
 * @param additional - Additional metadata to add to the end of the file
 * @returns Promise that resolves when metadata is updated
 */
const updateCommentMetadata = async (
  file: PathLike,
  metadata: Metadata[],
  expectationType: MetadataType,
  additional: string[] = [],
): Promise<void> => {
  if (!metadata.length) {
    return;
  }

  const fileContents = await readFile(file);
  const lines = fileContents.split('\n');

  metadata.forEach(({ type, line, value }) => {
    if (type !== expectationType) {
      abort(`Sanity check failed. Invalid metadata type ${type} for updating. Expected ${expectationType}`);
    }

    const match = lines[line - 1].match(/(.*)\/\/\s*\[(.+?)\] (.*)/);
    if (match) {
      const [, prefix, ,] = match;
      lines[line - 1] = `${prefix}// [${type}] ${value}`;
    } else
      throw new Error(
        `Failed to update metadata in ${file}. Line ${line} does not contain metadata: ${lines[line - 1]}`,
      );
  });

  additional.forEach(value => lines.push(`// [${expectationType}] ${value}`));

  await fs.writeFile(file, lines.join('\n'), { encoding: 'utf8' });
};
