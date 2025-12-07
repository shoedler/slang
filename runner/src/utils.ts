import chalk, { ChalkInstance } from 'chalk';
import { spawn } from 'node:child_process';
import { existsSync, PathLike } from 'node:fs';
import fs from 'node:fs/promises';
import path from 'node:path';
import process from 'node:process';
import wrapAnsi from 'wrap-ansi';
import { SLANG_PROJ_DIR, SlangBuildConfigs, SlangPaths, SlangRunFlags } from './config.ts';

/**
 * Run a command and return stdout. Exits if the process fails (non-zero exit code or stderr).
 * Aborts gracefully if the signal is aborted.
 * @param cmd - Command to run
 * @param errorMessage - Error message to throw if command fails (non-zero exit code or stderr)
 * @param signal - Abort signal to use
 * @param abortOnError - Whether to abort (exiting the app) on error (non-zero exit code or stderr)
 * @param ignoreStderr - Whether to ignore stderr and not abort on error
 * @returns Promise that resolves to an obj containing stdout, or undefined if abortOnError is false and the process fails - and the exit code
 */
export const runProcess = (
  cmd: string,
  errorMessage?: string,
  signal: AbortSignal | null = null,
  abortOnError = true,
  ignoreStderr = false,
): Promise<{
  output: string | undefined;
  code: number | null;
}> => {
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
        abort(errorMessage ?? err.message, err.toString()); // That's a no-no, we'll abort here.
      }
    });
    child.on('close', (code, nodeSignal) => {
      if (nodeSignal === 'SIGTERM') {
        return resolve({ output, code });
      }

      if (code === 0 && errorOutput === '') {
        return resolve({ output, code });
      } else {
        if (ignoreStderr && code === 0) {
          return resolve({ output, code });
        }

        if (abortOnError) {
          abort(
            errorMessage ?? 'Process had non-zero exit or output on stderr',
            `Command: ${cmd}, exit code: ${code}, stderr: ${errorOutput}, stdout: ${output}`,
          );
        } else {
          error(
            errorMessage ?? 'Process had non-zero exit or output on stderr',
            `Command: ${cmd}, exit code: ${code}, stderr: ${errorOutput}, stdout: ${output}`,
          );
          resolve({ output: undefined, code });
        }
      }
    });
  });
};

/**
 * Run a slang file with a given config. Collects stdout and stderr and returns it along with the
 * exit code. Aborts gracefully if the signal is aborted.
 * @param file - Absolute path to slang file to run
 * @param buildConfig - Build config to use
 * @param runFlags - Flags to pass to slang run
 * @param signal - Abort signal to use
 * @returns Object containing output and exit code
 */
export const runSlangFile = (
  file: PathLike,
  buildConfig: SlangBuildConfigs,
  runFlags: SlangRunFlags[] = [],
  signal: AbortSignal | null = null,
): Promise<{ exitCode: number | null; stdoutOutput: string; stderrOutput: string }> => {
  const binaryExt = process.platform === 'win32' ? '.exe' : '';
  const cmd = `${path.join(SlangPaths.BinDir, buildConfig, 'slang' + binaryExt)} run ${runFlags.join(' ')} ${file}`;
  const options = signal ? { signal } : {};
  const child = spawn(cmd, { shell: true, ...options });

  return new Promise(resolve => {
    let stdoutOutput = '';
    let stderrOutput = '';

    child.stdout.on('data', data => (stdoutOutput += data.toString()));
    child.stderr.on('data', data => (stderrOutput += data.toString()));
    child.on('error', err => {
      if (err.name === 'AbortError') {
        info('Aborting process...', cmd);
      } else {
        abort(err.toString()); // That's a no-no, we'll abort here.
      }
    });

    child.on('close', exitCode => {
      return resolve({
        stdoutOutput,
        stderrOutput,
        exitCode,
      });
    });
  });
};

/**
 * Delete a directory recursively. Retries forever (if access is denied or file is busy) until
 * the directory is deleted or the signal is aborted. Exits in case of any other error.
 * @param dirPath - Absolute path to directory to delete
 * @param signal - Abort signal to use
 * @returns Promise that resolves when directory is deleted
 */
const forceDeleteDirectory = async (dirPath: PathLike, signal: AbortSignal | null): Promise<void> => {
  while (true) {
    try {
      await fs.rm(dirPath, { recursive: true, force: true });
      break;
    } catch (err: unknown) {
      const error = err as NodeJS.ErrnoException;
      if (signal?.aborted) {
        return;
      } else if (error.code === 'EBUSY' || error.code === 'EPERM' || error.code === 'EACCES') {
        debug(`Waiting to delete ${dirPath}`, `Got ${error.code}, retrying in a bit...`);
        await new Promise(resolve => setTimeout(resolve, 200));
      } else {
        abort(`Failed to delete ${dirPath}`, error.toString());
      }
    }
  }
};

/**
 * Build slang with a given config
 * @param buildConfig - Build config to use
 * @param signal - Abort signal to use
 * @param abortOnError - Whether to abort (exiting the app) on error (non-zero exit code or stderr)
 * @param extraMakeArgs - Extra arguments to pass to make
 * @returns Promise that resolves when build completes with the output and exit code
 */
export const buildSlangConfig = async (
  buildConfig: SlangBuildConfigs,
  signal: AbortSignal | null = null,
  abortOnError = true,
  extraMakeArgs: string = '',
): Promise<{
  output: string | undefined;
  code: number | null;
}> => {
  const cmd = `make -C ${SLANG_PROJ_DIR} ${buildConfig} ${extraMakeArgs}`;
  await forceDeleteDirectory(path.join(SlangPaths.BinDir, buildConfig), signal);
  info(`Building slang ${buildConfig}`, `Command: "${cmd}"`);
  return await runProcess(cmd, `Building config "${buildConfig}" failed`, signal, abortOnError);
};

/**
 * Tests if flagName flag in common.h is enabled or disabled (commented out).
 * @param flagName - Name of the flag to test
 * @returns Promise that resolves to true if flagName is enabled, false otherwise
 */
export const testFeatureFlag = async (flagName: string): Promise<boolean> => {
  const commonFile = path.join(SLANG_PROJ_DIR, 'common.h');
  const commonContents = await readFile(commonFile);
  const disabled = new RegExp(`^\\s*\\/\\/\\s*#define\\s+${flagName}`, 'm').test(commonContents);
  const enabled = new RegExp(`^\\s*#define\\s+${flagName}`, 'm').test(commonContents);
  if (!disabled && !enabled) {
    abort(`Failed to test ${flagName} flag`, `common.h does not contain ${flagName}`);
  }
  return enabled;
};

/**
 * Finds files in a directory with a given suffix recursively
 * @param dir - Directory to search
 * @param suffix - File suffix to match
 * @returns Promise that resolves to an array of absolute paths to files or an empty array if no files are found
 */
export const findFiles = async (dir: PathLike, suffix: string): Promise<string[]> => {
  const files = [];
  const dirs = [dir];
  while (dirs.length > 0) {
    const d = dirs.pop();
    if (!d) {
      continue;
    }

    const contents = await fs.readdir(d);
    for (const c of contents) {
      const p = path.join(d.toString(), c);
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
 * @param file - Absolute path to file to create or append to
 * @param data - Data to append to file
 */
export const createOrAppendJsonFile = async (file: PathLike, data: unknown[]) => {
  let newArray = data;
  if (existsSync(file)) {
    const existingArray = JSON.parse(await fs.readFile(file, 'utf-8')) || [];
    newArray = [...existingArray, ...data];
  }

  let content = '[\n';
  content += newArray.map(i => '  ' + JSON.stringify(i)).join(',\n');
  content += '\n]\n';

  await fs.writeFile(file, content, { encoding: 'utf-8', flag: 'w' });
};

/**
 * Read a file and return it
 * @param file - Absolute path to file to read
 * @returns Promise that resolves to the file contents
 */
export const readFile = async (file: PathLike): Promise<string> => {
  return await fs.readFile(file, 'utf-8');
};

/**
 * Get git status of the current repo (commit hash, date, and message)
 * @returns Promise that resolves to git status
 */
export const gitStatus = async (): Promise<{ date: string; hash: string; message: string }> => {
  const formats = ['%ci', '%H', '%s'];
  const cmd = `git log -1 --pretty=format:`;
  const [date, hash, message] = await Promise.all(
    formats.map(f => runProcess(cmd + f, `Getting git log faied`).then(o => o.output)),
  );
  return { date: date!, hash: hash!, message: message! };
};

/**
 * Get the name of the processor
 * @returns Promise that resolves to the processor name
 */
export const getProcessorName = async (): Promise<string> => {
  let cmd: string;
  let parseOutput: (output: string) => string;

  if (process.platform === 'win32') {
    cmd = 'wmic cpu get name';
    parseOutput = output => output.split('\r\n')[1]?.trim() || 'COULD NOT GET PROCESSOR NAME';
  } else if (process.platform === 'linux') {
    cmd = 'cat /proc/cpuinfo | grep "model name" | head -n1 | cut -d: -f2';
    parseOutput = output => output.trim() || 'COULD NOT GET PROCESSOR NAME';
  } else if (process.platform === 'darwin') {
    cmd = 'sysctl -n machdep.cpu.brand_string';
    parseOutput = output => output.trim() || 'COULD NOT GET PROCESSOR NAME';
  } else {
    return 'UNSUPPORTED PLATFORM';
  }

  info(`Getting processor name`, `Command: "${cmd}"`);
  const output = (await runProcess(cmd, `Getting processor name failed`, null, false, true)).output ?? '';
  return parseOutput(output);
};

type LogType = 'err' | 'abort' | 'info' | 'ok' | 'warn' | 'debug' | 'pass' | 'skip' | 'fail' | 'done' | 'next';

/**
 * Object containing log configuration.
 * Each value is an array with the following elements:
 * [0] - Prefix for the log message (header), e.g. '█ Error'
 * [1] - chalk style for the header
 * [2] - Prefix for multiline header, e.g. '│      '
 * [3] - chalk style for multiline header
 */
// prettier-ignore
export const LOG_CONFIG: Record<LogType, [string, ChalkInstance, string, ChalkInstance]> = {
  err:   [ '█ Error', chalk.red.bold,              '│      ', chalk.red.bold           ],
  abort: [ '█ Abort', chalk.red.bold,              '│      ', chalk.red.bold           ],
  info:  [ '█ Info ', chalk.gray.bold,             '│      ', chalk.gray.bold          ],
  ok:    [ '█ Ok   ', chalk.green.bold,            '│      ', chalk.green.bold         ],
  warn:  [ '█ Warn ', chalk.yellowBright.bold,     '│      ', chalk.yellowBright.bold  ],
  debug: [ '█ Debug', chalk.magentaBright.bold,    '│      ', chalk.magentaBright.bold ],
  pass:  [ ' Pass ',  chalk.bgGreen.black,         '│     ',  chalk.green              ],
  skip:  [ ' skip ',  chalk.bgBlue.white,          '│     ',  chalk.blue               ],
  fail:  [ ' Fail ',  chalk.bgRed.white,           '│     ',  chalk.red                ],
  done:  [ ' Done ',  chalk.bgGreen.black,         '│     ',  chalk.green              ],
  next:  [ '->',      chalk.magentaBright.bold,    '│     ',  chalk.magentaBright      ]
}

/**
 * Prints a separator
 */
export const separator = () => console.log(chalk.gray('─'.repeat(80)));

/**
 * Log a styled message to the console.
 * @param type - Type of message
 * @param message - Message
 */
const prettyPrint = (type: LogType, message: string) => {
  const [headerPrefix, headerStyle, multilineHeaderValue, multilineHeaderStyle] = LOG_CONFIG[type];

  const maxWidth = process.stdout.columns - 1;
  const headerSpace = ' ';
  const availableWidth = maxWidth - headerSpace.length - headerPrefix.length;

  const wrapped = wrapAnsi(message, availableWidth, {
    trim: false,
    hard: true,
    wordWrap: true,
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
 * @param message - Error message to print
 * @param hint - Hint to print
 */
export const error = (message: string, hint?: string) => {
  prettyPrint('err', message + (hint ? chalk.gray(` ${hint}`) : ''));
};

const ABORT_HANDLERS: Array<() => void> = [];

/**
 * Add an abort handler. Gets called when the process is aborted using @see abort
 * @param handler - Handler to add
 */
export const addAbortHandler = (handler: () => void) => {
  ABORT_HANDLERS.push(handler);
};

/**
 * Abort the process with an error message
 * @param message - Error message to print
 * @param hint - Hint to print
 */
export const abort = (message: string, hint?: string) => {
  prettyPrint('abort', message + (hint ? chalk.gray(` ${hint}`) : ''));
  for (const handler of ABORT_HANDLERS) {
    handler();
  }
  process.exit(1);
};

/**
 * Print an info message
 * @param message - Info message to print
 * @param hint - Hint to print
 */
export const info = (message: string, hint?: string) => {
  prettyPrint('info', message + (hint ? chalk.gray(` ${hint}`) : ''));
};

/**
 * Print an ok message
 * @param message - Ok message to print
 * @param hint - Hint to print
 */
export const ok = (message: string, hint?: string) => {
  prettyPrint('ok', message + (hint ? chalk.gray(` ${hint}`) : ''));
};

/**
 * Print a warning message
 * @param message - Warning message to print
 * @param hint - Hint to print
 */
export const warn = (message: string, hint?: string) => {
  prettyPrint('warn', message + (hint ? chalk.gray(` ${hint}`) : ''));
};

/**
 * Print a debug message
 * @param message - Debug message to print
 * @param hint - Hint to print
 */
export const debug = (message: string, hint?: string) => {
  prettyPrint('debug', message + (hint ? chalk.gray(` ${hint}`) : ''));
};

/**
 * Print a test success message
 * @param message - Message to append
 */
export const pass = (message: string) => {
  prettyPrint('pass', message);
};

/**
 * Print a test failure message
 * @param message - Message to append
 */
export const fail = (message: string) => {
  prettyPrint('fail', message);
};

/**
 * Print a test skip message
 * @param message - Message to append
 */
export const skip = (message: string) => {
  prettyPrint('skip', message);
};
