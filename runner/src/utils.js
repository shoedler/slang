import chalk from "chalk";
import { spawn } from "node:child_process";
import { existsSync } from "node:fs";
import fs from "node:fs/promises";
import path from "node:path";
import { MSBUILD_EXE, SLANG_BIN_DIR, SLANG_PROJ_DIR } from "./config.js";

/**
 * Run a command and return stdout. Exits if the process fails (non-zero exit code or stderr).
 * Aborts gracefully if the signal is aborted.
 * @param {string} cmd - Command to run
 * @param {string} errorMessage - Error message to throw if command fails (non-zero exit code or stderr)
 * @param {AbortSignal} signal - Abort signal to use
 * @returns {Promise<string>} - Promise that resolves to stdout
 */
export const runProcess = (cmd, errorMessage, signal = null) => {
  const options = signal ? { signal } : {};
  const child = spawn(cmd, { shell: true, ...options });

  return new Promise((resolve) => {
    let output = "";
    let error = "";

    child.stdout.on("data", (data) => (output += data.toString()));
    child.stderr.on("data", (data) => (error += data.toString()));
    child.on("error", (err) => {
      if (err.name === "AbortError") {
        info("Aborting process...", cmd);
      } else {
        exitWithError(errorMessage, err);
      }
    });
    child.on("close", (code, nodeSignal) => {
      if (nodeSignal === "SIGTERM") {
        return resolve(output);
      }

      if (code === 0 && error === "") {
        return resolve(output);
      } else {
        exitWithError(errorMessage, `Exit code: ${code}, stderr: ${error}, stdout: ${output}`);
      }
    });
  });
};

/**
 * Get the path to the slang executable for a given config
 * @param {string} config - Build config to use
 * @returns
 */
export const getSlangExe = (config) => path.join(SLANG_BIN_DIR, config, "Slang.exe");

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

  return new Promise((resolve) => {
    let output = "";

    child.stdout.on("data", (data) => (output += data.toString()));
    child.stderr.on(
      "data",
      (data) => (output += colorStderr ? chalk.red(data.toString()) : data.toString())
    );
    child.on("error", (err) => {
      if (err.name === "AbortError") {
        info("Aborting process...", cmd);
      } else {
        exitWithError(err);
      }
    });

    child.on("close", (exitCode) => {
      return resolve({
        rawOutput: output,
        output: output
          .split("\r\n")
          .filter(Boolean)
          .map((line) => line.trim()),
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
      } else if (err.code === "EBUSY" || err.code === "EPERM" || err.code === "EACCES") {
        debug(`Failed to delete ${dirPath}`, `${err.code}, retrying in a bit...`);
        await new Promise((resolve) => setTimeout(resolve, 200));
      } else {
        exitWithError(`Failed to delete ${dirPath}`, err);
      }
    }
  }
};

/**
 * Build slang with a given config
 * @param {string} config - Build config to use
 * @param {AbortSignal} signal - Abort signal to use
 * @returns {Promise<string>} - Promise that resolves when build completes
 */
export const buildSlangConfig = async (config, signal = null) => {
  const cmd = `cd ${SLANG_PROJ_DIR} && ${MSBUILD_EXE} Slang.sln /p:Configuration=${config}`;
  await forceDeleteDirectory(path.join(SLANG_BIN_DIR, config), signal);
  info(`Building slang ${config}`, `Command: "${cmd}"`);
  return await runProcess(cmd, `Building config "${config}" failed`, signal);
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
 * Create or append to a JSON file. If the file already exists, the existing JSON is merged with
 * the new JSON using the append function.
 * @param {string} file - Absolute path to file to create or append to
 * @param {any} data - Data to append to file
 * @param {(existingJsonObj: any, newJsonObj: any) => any} append - Function to merge existing JSON with new JSON
 */
export const createOrAppendJsonFile = async (file, data, append) => {
  let newJsonObj = data;
  if (existsSync(file)) {
    const existingJsonObj = JSON.parse(await fs.readFile(file, "utf-8"));
    newJsonObj = append(existingJsonObj, newJsonObj);
  }

  await fs.writeFile(file, JSON.stringify(newJsonObj, null, 2), { encoding: "utf-8", flag: "w" });
};

/**
 * Parses a slang file and returns comment-based metadata.
 * Metadata is contained in a line comment (//) and is surrounded by brackets.
 * Metadata can have a value which is everything to the right of the closing bracket.
 * @param {string} file - Absolute path to a slang file
 * @returns {{type: string, line: number, value: string}[]} - Array containing metadata
 */
export const extractCommentMetadata = async (file) => {
  const fileContents = await fs.readFile(file, "utf8");
  const lines = fileContents.split("\n");
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
 * Abort the process with an error message
 * @param {string} message - Error message to print
 * @param {string} hint - Hint to print
 */
export const exitWithError = (message, hint) => {
  console.error(chalk.red.bold("▪ Error ") + message + (hint ? chalk.gray(` (${hint})`) : ""));
  process.exit(1);
};

/**
 * Print an info message
 * @param {string} message - Info message to print
 * @param {string} hint - Hint to print
 */
export const info = (message, hint) => {
  console.log(chalk.gray.bold("▪ Info  ") + message + (hint ? chalk.gray(` (${hint})`) : ""));
};

/**
 * Print a warning message
 * @param {string} message - Warning message to print
 * @param {string} hint - Hint to print
 */
export const warn = (message, hint) => {
  console.warn(
    chalk.yellowBright.bold("▪ Warn  ") + message + (hint ? chalk.gray(` (${hint})`) : "")
  );
};

/**
 * Print a debug message
 * @param {string} message - Debug message to print
 * @param {string} hint - Hint to print
 */
export const debug = (message, hint) => {
  console.debug(
    chalk.magentaBright.bold("▪ Debug ") + message + (hint ? chalk.gray(` (${hint})`) : "")
  );
};

/**
 * Print a test success message
 * @param {string} message - Message to append
 */
export const pass = (message) => {
  console.log(" " + chalk.bgGreen.black(" Pass ") + " " + chalk.gray(message));
};

/**
 * Print a test failure message
 * @param {string} message - Message to append
 */
export const fail = (message) => {
  console.log(" " + chalk.bgRed.black(" Fail ") + " " + chalk.gray(message));
};

/**
 * Prints a separator
 */
export const separator = () => console.log(chalk.gray("-".repeat(30)));
