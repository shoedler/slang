import chalk from 'chalk';
import { spawn } from 'node:child_process';
import { existsSync } from 'node:fs';
import fs from 'node:fs/promises';
import path from 'node:path';
import wrapAnsi from 'wrap-ansi';
import { MSBUILD_EXE, SLANG_BIN_DIR, SLANG_PROJ_DIR } from './config.js';

/**
 * Run a command and return stdout. Exits if the process fails (non-zero exit code or stderr).
 * Aborts gracefully if the signal is aborted.
 * @param {string} cmd - Command to run
 * @param {string} errorMessage - Error message to throw if command fails (non-zero exit code or stderr)
 * @param {AbortSignal} signal - Abort signal to use
 * @param {boolean} abortOnError - Whether to abort (exiting the app) on error (non-zero exit code or stderr)
 * @param {boolean} ignoreStderr - Whether to ignore stderr and not abort on error
 * @returns {Promise<string | undefined>} - Promise that resolves to stdout or undefined if abortOnError is false and the process fails
 */
export const runProcess = (
  cmd,
  errorMessage,
  signal = null,
  abortOnError = true,
  ignoreStderr = false,
) => {
  const options = signal ? { signal } : {};
  const child = spawn(cmd, { shell: true, ...options });

  return new Promise(resolve => {
    let output = '';
    let errorOutput = '';

    child.stdout.on('data', data => (output += data.toString()));
    child.stderr.on('data', data => (errorOutput += data.toString()));
    child.on('error', err => {
      if (err.name === 'AbortError') {
        info('Aborting process...', cmd);
      } else {
        abort(errorMessage, err); // That's a no-no, we'll abort here.
      }
    });
    child.on('close', (code, nodeSignal) => {
      if (nodeSignal === 'SIGTERM') {
        return resolve(output);
      }

      if (code === 0 && errorOutput === '') {
        return resolve(output);
      } else {
        if (ignoreStderr && code === 0) {
          return resolve(output);
        }

        if (abortOnError) {
          abort(errorMessage, `Exit code: ${code}, stderr: ${errorOutput}, stdout: ${output}`);
        } else {
          error(errorMessage, `Exit code: ${code}, stderr: ${errorOutput}, stdout: ${output}`);
          resolve(undefined);
        }
      }
    });
  });
};

/**
 * Get the path to the slang executable for a given config
 * @param {string} config - Build config to use
 * @returns
 */
export const getSlangExe = config => path.join(SLANG_BIN_DIR, config, 'slang.exe');

/**
 * Make a command to run a slang file with a given config
 * @param {string} config - Build config to use
 * @param {string} file - Absolute path to slang file to run
 * @returns {string} - Command to run slang file
 */
const makeSlangRunCommand = (config, file) => `${getSlangExe(config)} run ${file}`;

/**
 * Run a slang file with a given config. Collects stdout and stderr and returns it along with the
 * exit code. Aborts gracefully if the signal is aborted.
 * @param {string} config - Build config to use
 * @param {string} file - Absolute path to slang file to run
 * @param {AbortSignal} signal - Abort signal to use
 * @param {boolean} colorStderr - Whether to color stderr red
 * @returns {{output: string[], exitCode: number, rawOutput: string }} - Object containing output and exit code
 */
export const runSlangFile = async (file, config, signal = null, colorStderr = false) => {
  const cmd = makeSlangRunCommand(config, file);
  const options = signal ? { signal } : {};
  const child = spawn(cmd, { shell: true, ...options });

  return new Promise(resolve => {
    let output = '';

    child.stdout.on('data', data => (output += data.toString()));
    child.stderr.on(
      'data',
      data => (output += colorStderr ? chalk.red(data.toString()) : data.toString()),
    );
    child.on('error', err => {
      if (err.name === 'AbortError') {
        info('Aborting process...', cmd);
      } else {
        abort(err); // That's a no-no, we'll abort here.
      }
    });

    child.on('close', exitCode => {
      return resolve({
        rawOutput: output,
        output: output
          .split('\r\n')
          .filter(Boolean)
          .map(line => line.trim()),
        exitCode,
      });
    });
  });
};

/**
 * Delete a directory recursively. Retries forever (if access is denied or file is busy) until
 * the directory is deleted or the signal is aborted. Exits in case of any other error.
 * @param {string} dirPath - Absolute path to directory to delete
 * @param {AbortSignal} signal - Abort signal to use
 * @returns {Promise<void>} - Promise that resolves when directory is deleted
 */
const forceDeleteDirectory = async (dirPath, signal) => {
  while (true) {
    try {
      await fs.rm(dirPath, { recursive: true, force: true, signal });
      break;
    } catch (err) {
      if (signal?.aborted) {
        return;
      } else if (err.code === 'EBUSY' || err.code === 'EPERM' || err.code === 'EACCES') {
        debug(`Waiting to delete ${dirPath}`, `Got ${err.code}, retrying in a bit...`);
        await new Promise(resolve => setTimeout(resolve, 200));
      } else {
        abort(`Failed to delete ${dirPath}`, err);
      }
    }
  }
};

/**
 * Build slang with a given config
 * @param {string} config - Build config to use
 * @param {AbortSignal} signal - Abort signal to use
 * @param {boolean} abortOnError - Whether to abort (exiting the app) on error (non-zero exit code or stderr)
 * @returns {Promise<string>} - Promise that resolves when build completes
 */
export const buildSlangConfig = async (config, signal = null, abortOnError = true) => {
  const cmd = `cd ${SLANG_PROJ_DIR} && ${MSBUILD_EXE} slang.sln /p:Configuration=${config}`;
  await forceDeleteDirectory(path.join(SLANG_BIN_DIR, config), signal);
  info(`Building slang ${config}`, `Command: "${cmd}"`);
  return await runProcess(cmd, `Building config "${config}" failed`, signal, abortOnError);
};

/**
 * Tests if DEBUG_STRESS_GC flag in common.h is enabled or disabled (commented out).
 * @param {boolean} enabled - Whether to ensure the flag is enabled or disabled
 * @returns {Promise<boolean>} - Promise that resolves to true if DEBUG_STRESS_GC is configured correctly
 */
export const testGcStressFlag = async enabled => {
  const commonFile = path.join(SLANG_PROJ_DIR, 'common.h');
  const commonContents = await readFile(commonFile);
  const regex = enabled ? /^\s*\/\/\s*#define\s+DEBUG_STRESS_GC/ : /^\s*#define\s+DEBUG_STRESS_GC/;
  return regex.test(commonContents);
};

/**
 * Finds files in a directory with a given suffix recursively
 * @param {string} dir - Directory to search
 * @param {string} suffix - File suffix to match
 */
export const findFiles = async (dir, suffix) => {
  const files = [];
  const dirs = [dir];
  while (dirs.length > 0) {
    const d = dirs.pop();
    const contents = await fs.readdir(d);
    for (const c of contents) {
      const p = path.join(d, c);
      const stat = await fs.stat(p);
      if (stat.isDirectory()) {
        dirs.push(p);
      } else if (stat.isFile() && p.endsWith(suffix)) {
        files.push(p);
      }
    }
  }
  return files;
};

/**
 * Create a, or append items to a JSON-array file. If the file already exists, the new items are appended.
 * Will print every item as a single line in the file.
 * @param {string} file - Absolute path to file to create or append to
 * @param {any[]} array - Data to append to file
 */
export const createOrAppendJsonFile = async (file, array) => {
  let newArray = array;
  if (existsSync(file)) {
    const existingArray = JSON.parse(await fs.readFile(file, 'utf-8')) || [];
    newArray = [...existingArray, ...array];
  }

  let content = '[\n';
  content += newArray.map(i => '  ' + JSON.stringify(i)).join(',\n');
  content += '\n]\n';

  await fs.writeFile(file, content, { encoding: 'utf-8', flag: 'w' });
};

/**
 * Read a file and return it
 * @param {string} file - Absolute path to file to read
 * @returns {Promise<any>} - Promise that resolves to the file contents
 */
export const readFile = async file => {
  return await fs.readFile(file, 'utf-8');
};

/**
 * Parses a slang file and returns comment-based metadata.
 * Metadata is contained in a line comment (//) and is surrounded by brackets.
 * Metadata can have a value which is everything to the right of the closing bracket.
 * @param {string} file - Absolute path to a slang file
 * @returns {{type: string, line: number, value: string}[]} - Array containing metadata
 */
export const extractCommentMetadata = async file => {
  const fileContents = await readFile(file);
  const lines = fileContents.split('\n');
  const metadata = [];
  lines.forEach((l, i) => {
    const match = l.match(/\/\/\s*\[(.+?)\](.*)/);
    if (match) {
      const [_, type, value] = match;
      metadata.push({ type, line: i + 1, value: value.trim() });
    }
  });
  return metadata;
};

/**
 * Update comment-based metadata in a slang file. Only handles [Expect]
 * @param {string} file - Absolute path to a slang file
 * @param {{type: string, line: number, value: string}[]} metadata - Array containing the new metadata
 */
export const updateCommentMetadata = async (file, metadata) => {
  const fileContents = await readFile(file);
  const lines = fileContents.split('\n');
  metadata.forEach(({ type, line, value }) => {
    const match = lines[line - 1].match(/(.*)\/\/\s*\[(.+?)\](.*)/);
    if (match) {
      const [_, prefix, __, ___] = match;
      lines[line - 1] = `${prefix}// [${type}] ${value}`;
    } else
      throw new Error(
        `Failed to update metadata in ${file}. Line ${line} does not contain metadata: ${
          lines[line - 1]
        }`,
      );
  });
  await fs.writeFile(file, lines.join('\n'), 'utf8');
};

/**
 * Get git status of the current repo (commit hash, date, and message)
 * @returns {Promise<{date: string, hash: string, message: string}>} - Promise that resolves to git status
 */
export const gitStatus = async () => {
  const formats = ['%ci', '%H', '%s'];
  const cmd = `git log -1 --pretty=format:`;
  const [date, hash, message] = await Promise.all(
    formats.map(f => runProcess(cmd + f, `Getting git log faied`)),
  );
  return { date, hash, message };
};

/**
 * Get the name of the processor
 * @returns {Promise<string>} - Promise that resolves to the processor name
 */
export const getProcessorName = async () => {
  const cmd = 'wmic cpu get name';
  info(`Getting processor name`, `Command: "${cmd}"`);
  const labelAndName = await runProcess(cmd, `Getting processor name failed`);
  return labelAndName.split('\r\n')[1].trim();
};

/**
 * Prints a separator
 */
export const separator = () => console.log(chalk.gray('─'.repeat(80)));

/**
 * Object containing log configuration.
 * Each value is an array with the following elements:
 * [0] - Prefix for the log message (header), e.g. '█ Error'
 * [1] - chalk style for the header
 * [2] - Prefix for multiline header, e.g. '│      '
 * [3] - chalk style for multiline header
 */
// prettier-ignore
export const LOG_CONFIG = {
  err:   [ '█ Error', chalk.red.bold,           '│      ', chalk.red.bold           ],
  abort: [ '█ Abort', chalk.red.bold,           '│      ', chalk.red.bold           ],
  info:  [ '█ Info ', chalk.gray.bold,          '│      ', chalk.gray.bold          ],
  ok:    [ '█ Ok   ', chalk.green.bold,         '│      ', chalk.green.bold         ],
  warn:  [ '█ Warn ', chalk.yellowBright.bold,  '│      ', chalk.yellowBright.bold  ],
  debug: [ '█ Debug', chalk.magentaBright.bold, '│      ', chalk.magentaBright.bold ],
  pass:  [ ' Pass ',  chalk.bgGreen.black,      '│     ',  chalk.green              ],
  fail:  [ ' Fail ',  chalk.bgRed.black,        '│     ',  chalk.red                ],
}

/**
 * Log a styled message to the console.
 * @param {'err' | 'warn' | 'debug' | 'info' | 'ok'} type - Type of message
 * @param {string} message - Message
 */
const prettyPrint = (type, message) => {
  const [headerPrefix, headerStyle, multilineHeaderValue, multilineHeaderStyle] = LOG_CONFIG[type];

  const maxWidth = process.stdout.columns - 1;
  const headerSpace = ' ';
  let availableWidth = maxWidth - headerSpace.length - headerPrefix.length;

  const wrapped = wrapAnsi(message, availableWidth, {
    trim: false,
    hard: true,
    wordWrap: true,
    wordWrapWidth: availableWidth,
  }).split('\n');

  for (let i = 0; i < wrapped.length; i++) {
    const line = wrapped[i];
    if (i === 0) {
      console.log(headerStyle(headerPrefix) + headerSpace + line);
    } else {
      console.log(multilineHeaderStyle(multilineHeaderValue) + headerSpace + line);
    }
  }
};

/**
 * Print an error message
 * @param {string} message - Error message to print
 * @param {string} hint - Hint to print
 */
export const error = (message, hint) => {
  prettyPrint('err', message + (hint ? chalk.gray(` (${hint})`) : ''));
};

/**
 * Abort the process with an error message
 * @param {string} message - Error message to print
 * @param {string} hint - Hint to print
 */
export const abort = (message, hint) => {
  prettyPrint('abort', message + (hint ? chalk.gray(` (${hint})`) : ''));
  process.exit(1);
};

/**
 * Print an info message
 * @param {string} message - Info message to print
 * @param {string} hint - Hint to print
 */
export const info = (message, hint) => {
  prettyPrint('info', message + (hint ? chalk.gray(` (${hint})`) : ''));
};

/**
 * Print an ok message
 * @param {string} message - Ok message to print
 * @param {string} hint - Hint to print
 */
export const ok = (message, hint) => {
  prettyPrint('ok', message + (hint ? chalk.gray(` (${hint})`) : ''));
};

/**
 * Print a warning message
 * @param {string} message - Warning message to print
 * @param {string} hint - Hint to print
 */
export const warn = (message, hint) => {
  prettyPrint('warn', message + (hint ? chalk.gray(` (${hint})`) : ''));
};

/**
 * Print a debug message
 * @param {string} message - Debug message to print
 * @param {string} hint - Hint to print
 */
export const debug = (message, hint) => {
  prettyPrint('debug', message + (hint ? chalk.gray(` (${hint})`) : ''));
};

/**
 * Print a test success message
 * @param {string} message - Message to append
 */
export const pass = message => {
  prettyPrint('pass', message);
};

/**
 * Print a test failure message
 * @param {string} message - Message to append
 */
export const fail = message => {
  prettyPrint('fail', message);
};
