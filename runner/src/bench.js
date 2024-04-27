import {
  BENCH_LOG_FILE,
  BENCH_SUFFIX,
  BUILD_CONFIG_RELEASE,
  SLANG_BENCH_DIR,
  SLANG_BIN_DIR,
  SLANG_SUFFIX,
} from './config.js';
import {
  LOG_CONFIG,
  createOrAppendJsonFile,
  getProcessorName,
  gitStatus,
  info,
  ok,
  readFile,
  runProcess,
  warn,
} from './utils.js';

import chalk from 'chalk';
import { existsSync } from 'node:fs';
import http from 'node:http';
import path from 'node:path';

/**
 * @typedef {{
 *   name: string;
 *   cpu: string;
 *   lang: string;
 *   v: string;
 *   date: Date;
 *   score: number;
 *   score: number;
 *   best: number;
 *   avg: number;
 *   worst: number;
 *   sd: number;
 * }} BenchmarkResult
 */

const LANGUAGES = [
  {
    lang: 'slang',
    ext: SLANG_SUFFIX,
    cmd: [`${path.join(SLANG_BIN_DIR, BUILD_CONFIG_RELEASE, 'slang')}`, 'run'],
    version: undefined,
  },
  {
    lang: 'javascript',
    ext: '.js',
    cmd: ['node', '--jitless', '--noexpose_wasm'],
    version: ['node', '--version'],
  },
  {
    lang: 'python',
    ext: '.py',
    cmd: ['py'],
    version: ['py', '--version'],
  },
];

const BENCHMARKS = [];

/**
 * Define a benchmark
 * @param {string} name - The name of the benchmark (file name, without extension and .bench suffix)
 * @param {string[]} expectedOutput - The expected output of the benchmark
 */
const defineBenchmark = (name, expectedOutput) => {
  const expectedOutStr = expectedOutput.length > 0 ? expectedOutput.join('\n') + '\n' : '';
  const regex = new RegExp(expectedOutStr + 'elapsed: (\\d+.\\d+)', 'm');
  BENCHMARKS.push({ name, regex });
};

defineBenchmark('zoo', ['1800000', '1800000', '1800000', '1800000', '1800000']);
defineBenchmark('string', []);
defineBenchmark('fib', ['832040', '832040', '832040', '832040', '832040']);
defineBenchmark('list', ['499999500000']);

/**
 * Calculate score based on time
 * @param {number} time - Time to calculate score for
 * @returns {number} - Score
 */
const getScore = time => 1000 / time;

/**
 * Replace Carriage Return + Line Feed (\r\n) with Line Feed (\n)
 * Then replace any remaining Carriage Returns (\r) with Line Feed (\n)
 * @param {string} stdout - Output to normalize
 * @returns {string} - Normalized output
 */
const normalizeLineEndings = stdout => {
  return stdout.replace(/\r\n/g, '\n').replace(/\r/g, '\n');
};

/**
 * Calculate standard deviation of an array of numbers
 * @param {number[]} values - Array of numbers
 * @returns {number} - Standard deviation
 */
const standardDeviation = values => {
  const avg = values.reduce((a, b) => a + b) / values.length;
  const variance = values.reduce((a, b) => a + (b - avg) ** 2, 0) / values.length;
  return Math.sqrt(variance);
};

/**
 * Find a result in an array of results
 * @param {BenchmarkResult[]} results - Array of results
 * @param {string} name - Name of the benchmark
 * @param {string} lang - Language of the benchmark
 * @param {boolean} [first=true] - Find the first result or the last result
 * @returns {BenchmarkResult | undefined} - Result or undefined if not found
 */
const findResult = (results, name, lang, first = true) => {
  if (first) {
    return results.find(r => r.name === name && r.lang === lang);
  }
  for (let i = results.length - 1; i >= 0; i--) {
    if (results[i].name === name && results[i].lang === lang) {
      return results[i];
    }
  }
  return undefined;
};

/**
 * Runs all benchmarks and writes results to a log file
 * @param {string} langPattern - Pattern to match languages (regex)
 */
export const runBenchmarks = async langPattern => {
  const benchLogFile = path.join(SLANG_BENCH_DIR, BENCH_LOG_FILE);
  const prevResults = JSON.parse(await readFile(benchLogFile));
  const results = [];

  const date = new Date();
  const processorName = await getProcessorName();

  const actualLanguages = LANGUAGES.filter(l => l.lang.match(langPattern));

  const longestBenchmarkName = Math.max(...BENCHMARKS.map(b => b.name.length));
  const longestLanguageName = Math.max(...actualLanguages.map(l => l.lang.length));

  for (const benchmark of BENCHMARKS) {
    for (const language of actualLanguages) {
      const { lang, ext, cmd } = language;
      const filePath = path.join(SLANG_BENCH_DIR, benchmark.name + BENCH_SUFFIX + ext);
      const runCommand = cmd.join(' ') + ' ' + filePath;
      const isSlang = lang === 'slang';
      const times = [];

      let interpreterVersion = isSlang
        ? (await gitStatus()).hash
        : await runProcess(language.version.join(' '));

      interpreterVersion = normalizeLineEndings(interpreterVersion).replace(/\n/g, '');

      // Print benchmark header
      const [, , multilineHeader, multilineHeaderStyle] = LOG_CONFIG['info'];
      process.stdout.write(multilineHeaderStyle(multilineHeader));
      process.stdout.write(`  ${chalk.bold(benchmark.name)} `);
      process.stdout.write(' '.repeat(longestBenchmarkName - benchmark.name.length + 1));
      process.stdout.write(`${chalk.italic(lang == 'slang' ? chalk.magenta(lang) : lang)} `);
      process.stdout.write(' '.repeat(longestLanguageName - lang.length + 1));

      if (!existsSync(filePath)) {
        process.stdout.write(` (file ${filePath} not found)\n`);
        continue;
      }

      // Do one warmup run
      await runProcess(runCommand, '', undefined, false, true);

      // Run benchmark 10 times
      for (let i = 0; i < 10; i++) {
        process.stdout.write(`.`);

        const output = await runProcess(runCommand, '', undefined, false, true);

        if (!output) {
          process.stdout.write('\n');
          warn(`${lang} benchmark failed`, `Output: ${output}`);
          continue;
        }

        const match = normalizeLineEndings(output).trim().match(benchmark.regex);
        if (!match) {
          process.stdout.write('\n');
          warn(`${lang} benchmark output does not match expected output`, output);
          continue;
        }
        times.push(parseFloat(match[1]));
      }

      // Calculate some stats
      const best = Math.min(...times);
      const avg = times.reduce((a, b) => a + b) / times.length;
      const worst = Math.max(...times);
      const score = getScore(avg);
      let standardDev = standardDeviation(times);

      // Print benchmark results
      let comparison;
      let comparisonSuffix;
      let dividend = score;
      let divisor = score;

      if (isSlang) {
        // If we're running a slang benchmark, compare to a previous slang result
        const prevResult = findResult(prevResults, benchmark.name, 'slang', false);
        if (prevResult) {
          comparisonSuffix = '% relative to baseline';
          divisor = prevResult.score;
        } else {
          comparison = 'no baseline';
          comparisonSuffix = '';
        }
      } else {
        // If we're running a non-slang benchmark, compare to the slang result. Slang always runs first.
        const slangResult = findResult(results, benchmark.name, 'slang');
        if (slangResult) {
          comparisonSuffix = '%';
          dividend = slangResult.score;
        } else {
          comparison = 'no slang result';
          comparisonSuffix = '';
        }
      }

      // Calculate comparison
      const ratio = (100 * dividend) / divisor;
      if (!comparison) {
        comparison = ratio.toFixed(2) + comparisonSuffix;
      }
      if (ratio > 100) {
        comparison = chalk.green(comparison);
      }
      if (ratio < 100) {
        comparison = chalk.red(comparison);
      }

      // Emphasize standard deviation if it's high
      if (standardDev > 0.05) {
        standardDev = chalk.yellow(standardDev.toFixed(4));
      } else {
        standardDev = standardDev.toFixed(4);
      }

      process.stdout.write(` ${best.toFixed(2)}s sd(${standardDev}) ${comparison}\n`);

      // Push benchmark result to results array
      const result = {
        name: benchmark.name,
        cpu: processorName,
        date,
        lang,
        score,
        best,
        avg,
        worst,
        sd: standardDev,
        v: interpreterVersion,
      };

      results.push(result);
    }
  }

  // Write results to log file
  info(`Writing results.`, `to ${benchLogFile}`);
  await createOrAppendJsonFile(benchLogFile, results);

  ok(`All benchmarks done.`);
};

export const serveResults = async () => {
  const clientCodeFile = path.join(SLANG_BENCH_DIR, 'client.js');
  const indexHtmlFile = path.join(SLANG_BENCH_DIR, 'index.html');
  const benchLogFile = path.join(SLANG_BENCH_DIR, BENCH_LOG_FILE);

  const server = http.createServer((req, res) => {
    if (req.url === '/') {
      readFile(indexHtmlFile).then(indexHtml => {
        res.writeHead(200, { 'Content-Type': 'text/html' });
        res.end(indexHtml);
      });
    } else if (req.url === '/client') {
      readFile(clientCodeFile).then(clientCode => {
        res.writeHead(200, { 'Content-Type': 'application/javascript' });
        res.end(clientCode);
      });
    } else if (req.url === '/results') {
      readFile(benchLogFile).then(rawResults => {
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify(JSON.parse(rawResults), null, 2));
      });
    } else {
      res.writeHead(404, { 'Content-Type': 'text/plain' });
      res.end('Not found');
    }
  });

  server.listen(8080);
  info('Server running at http://localhost:8080/');
};
