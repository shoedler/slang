import chalk from 'chalk';
import { existsSync } from 'node:fs';
import http from 'node:http';
import path from 'node:path';
import process from 'node:process';
import { BENCH_LOG_FILE, BENCH_PRE_SUFFIX, SlangBuildConfigs, SlangFileSuffixes, SlangPaths } from './config.ts';
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
} from './utils.ts';

type BenchmarkResult = {
  name: string;
  cpu: string;
  lang: string;
  v: string;
  date: Date;
  score: number;
  best: number;
  avg: number;
  worst: number;
  sd: number;
};

type Benchmark = {
  name: string;
  regex: RegExp;
};

const NUM_RUNS = 20; // Number of runs for each benchmark

const LANGUAGES = [
  {
    lang: 'slang',
    ext: SlangFileSuffixes.Slang,
    cmd: [`${path.join(SlangPaths.BinDir, SlangBuildConfigs.Release, 'slang')}`, 'run'],
    version: [`${path.join(SlangPaths.BinDir, SlangBuildConfigs.Release, 'slang')}`, '--version'],
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

const BENCHMARKS: Benchmark[] = [];

/**
 * Define a benchmark
 * @param name - The name of the benchmark (file name, without extension and .bench suffix)
 * @param expectedOutput - The expected output of the benchmark, each output line as an array element
 */
const defineBenchmark = (name: string, expectedOutput: string[]) => {
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
 * @param time - Time to calculate score for
 * @returns Score
 */
const getScore = (time: number) => 1000 / time;

/**
 * Replaces all `\r\n` with `\n`, then replace any remaining `\r` with `\n`.
 * @param str - String to normalize
 * @returns Normalized output
 */
const normalizeLineEndings = (str: string) => {
  return str.replace(/\r\n/g, '\n').replace(/\r/g, '\n');
};

/**
 * Calculate standard deviation of an array of numbers
 * @param values - Array of numbers
 * @returns Standard deviation
 */
const standardDeviation = (values: number[]) => {
  const avg = values.reduce((a, b) => a + b) / values.length;
  const variance = values.reduce((a, b) => a + (b - avg) ** 2, 0) / values.length;
  return Math.sqrt(variance);
};

/**
 * Find a result in an array of results
 * @param results - Array of results
 * @param name - Name of the benchmark
 * @param lang - Language of the benchmark
 * @param cpu - CPU name running the benchmark
 * @param first - Find the first result or the last result
 * @returns Result or undefined if not found
 */
const findResult = (
  results: BenchmarkResult[],
  name: string,
  lang: string,
  cpu: string,
  first = true,
): BenchmarkResult | undefined => {
  const isTheOne = (r: BenchmarkResult) => r.name === name && r.lang === lang && r.cpu === cpu;

  if (first) {
    return results.find(isTheOne);
  }

  for (let i = results.length - 1; i >= 0; i--) {
    if (isTheOne(results[i])) {
      return results[i];
    }
  }

  return undefined;
};

/**
 * Runs all benchmarks and writes results to a log file
 * @param langPattern - Pattern to match languages (regex)
 */
export const runBenchmarks = async (langPattern?: string) => {
  const benchLogFile = path.join(SlangPaths.BenchDir, BENCH_LOG_FILE);
  const prevResults = JSON.parse(await readFile(benchLogFile));
  const results: BenchmarkResult[] = [];

  const date = new Date();
  const processorName = await getProcessorName();

  const actualLanguages = langPattern ? LANGUAGES.filter(l => l.lang.match(langPattern)) : LANGUAGES;

  const longestBenchmarkName = Math.max(...BENCHMARKS.map(b => b.name.length));
  const longestLanguageName = Math.max(...actualLanguages.map(l => l.lang.length));

  for (const language of actualLanguages) {
    const { lang, ext, cmd } = language;
    const isSlang = lang === 'slang';

    let interpreterVersion = await runProcess(language.version.join(' '));
    if (!interpreterVersion) {
      warn(`${lang} version command failed`, `Received no output`);
      interpreterVersion = 'unknown version';
    }

    if (isSlang) {
      interpreterVersion += ' @ ' + (await gitStatus()).hash;
    }

    interpreterVersion = normalizeLineEndings(interpreterVersion).replace(/\n/g, '');

    for (const benchmark of BENCHMARKS) {
      const filePath = path.join(SlangPaths.BenchDir, benchmark.name + BENCH_PRE_SUFFIX + ext);
      const runCommand = cmd.join(' ') + ' ' + filePath;
      const times: number[] = [];

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

      // Create a progress spinner
      const SPINNER = '⣾⣽⣻⢿⡿⣟⣯⣷';
      const [PENDING, COMPLETED] = '▰▰';
      const cursor = (show: boolean) => process.stdout.write(show ? '\x1b[?25h' : '\x1b[?25l');
      let i = 0;

      const spinner = setInterval(() => {
        cursor(false); // Hide cursor
        const char = SPINNER.charAt(Math.floor(Date.now() / 80) % SPINNER.length);

        process.stdout.write('\x1b[s'); // Save cursor position

        const progCurrent = char;
        const progDone = COMPLETED.repeat(i);
        const progRemain = PENDING.repeat(NUM_RUNS - i);

        const progress = chalk.green(progDone) + progCurrent + chalk.red(progRemain);

        const current = i.toString().padStart(NUM_RUNS.toString().length, ' ');
        process.stdout.write(` ${progress} ${current}/${NUM_RUNS}`);

        process.stdout.write('\x1b[u'); // Restore cursor position
      }, 100);

      const stopSpinner = () => {
        clearInterval(spinner);
        cursor(true); // Show cursor
      };

      // Do one warmup run
      for (let j = 0; j < NUM_RUNS; j++) await runProcess(runCommand, '', undefined, false, true);

      // Run benchmark
      for (; i < NUM_RUNS; i++) {
        const output = await runProcess(runCommand, '', undefined, false, true);

        if (!output) {
          stopSpinner();
          process.stdout.write('\n');
          warn(`${lang} running ${benchmark.name} benchmark failed`, `Received no output`);
          times.push(Infinity);
          break;
        }

        const match = normalizeLineEndings(output).trim().match(benchmark.regex);
        if (!match || !match[1] /* elapsed time */) {
          stopSpinner();
          process.stdout.write('\n');
          warn(`${lang} output for ${benchmark.name} does not match expected output`, output);
          times.push(Infinity);
          break;
        }

        times.push(parseFloat(match[1]));
      }

      stopSpinner();

      // Calculate some stats
      const best = Math.min(...times);
      const avg = times.reduce((a, b) => a + b) / times.length;
      const worst = Math.max(...times);
      const score = getScore(avg);
      const standardDev = standardDeviation(times);

      // Print benchmark results
      let comparison;
      let comparisonSuffix;
      let dividend = avg;
      let divisor = avg;

      if (isSlang) {
        // If we're running a slang benchmark, compare to a previous slang result
        const prevResult = findResult(prevResults, benchmark.name, 'slang', processorName, false);
        if (prevResult) {
          comparisonSuffix = '% relative to baseline';
          divisor = prevResult.avg;
        } else {
          comparison = 'no baseline for this benchmark on this cpu found';
          comparisonSuffix = '';
        }
      } else {
        // If we're running a non-slang benchmark, compare to the current slang result. Slang always runs first.
        const slangResult = findResult(results, benchmark.name, 'slang', processorName);
        if (slangResult) {
          comparisonSuffix = '%';
          dividend = slangResult.avg;
        } else {
          comparison = 'no slang result for this benchmark on this cpu found';
          comparisonSuffix = '';
        }
      }

      // Calculate comparison
      const ratio = (100 / dividend) * divisor;
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
      const avgStr = 'avg=' + chalk.bold(avg.toFixed(3)) + 's';
      const bestStr = 'best=' + best.toFixed(3) + 's';
      const worstStr = 'worst=' + worst.toFixed(3) + 's';
      const sdStr = 'sd=' + (standardDev > 0.05 ? chalk.yellow(standardDev.toFixed(4)) : standardDev.toFixed(4));

      process.stdout.write(` ${avgStr} ${bestStr} ${worstStr} ${sdStr} ${comparison}\n`);

      // Push benchmark result to results array
      const result: BenchmarkResult = {
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

/**
 * Serve benchmark results
 */
export const serveResults = () => {
  const clientCodeFile = path.join(SlangPaths.BenchDir, 'client.js');
  const indexHtmlFile = path.join(SlangPaths.BenchDir, 'index.html');
  const benchLogFile = path.join(SlangPaths.BenchDir, BENCH_LOG_FILE);

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

  server.listen(5252);
  info('Server running at http://localhost:5252/');
};

// const SPINNER = '⠁⠂⠄⡀⢀⠠⠐⠈';
// const SPINNER = '▊▋▌▍▎▏▎▍▌▋';
// const SPINNER = '⣾⣽⣻⢿⡿⣟⣯⣷';
// const SPINNER = '⊙⊚⊛⊜⊝';
// const SPINNER = '◰◱◲◳';
// const SPINNER = '◴◵◶◷';
// const SPINNER = '▖▘▝▗';
// const SPINNER = '✶✸✹✺✹✸';
// const SPINNER = '⎺⎻⎼⎽⎯';
// const SPINNER = '━┛┗━┏┓';
// const SPINNER = '◇◈◆◈';
// const SPINNER = '⯌⯐';
// const [PENDING, COMPLETED] = '▬▮';
// const [PENDING, COMPLETED] = '⬡⬢';
// const [PENDING, COMPLETED] = '▢▣';
// const [PENDING, COMPLETED] = '▢▣';
