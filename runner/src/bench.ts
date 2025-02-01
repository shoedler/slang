import chalk from 'chalk';
import { existsSync } from 'node:fs';
import http from 'node:http';
import path from 'node:path';
import process from 'node:process';
import { BENCH_LOG_FILE, BENCH_PRE_SUFFIX, SlangBuildConfigs, SlangFileSuffixes, SlangPaths } from './config.ts';
import {
  LOG_CONFIG,
  createOrAppendJsonFile,
  error,
  getProcessorName,
  gitStatus,
  info,
  ok,
  readFile,
  runProcess,
  warn,
} from './utils.ts';

type BenchmarkRun = {
  date: Date;
  score: number;
  best: number;
  avg: number;
  worst: number;
  sd: number;
};

export type BenchmarkResult = {
  name: string;
  cpu: string;
  lang: string;
  v: string;
} & BenchmarkRun;

type Benchmark = {
  name: string;
  numRuns: number;
  numWarmupRuns: number;
  regex: RegExp;
};

export type Language = {
  name: string;
  ext: `.${string}`;
  cmdRunFile: string[];
  cmdGetVersion: string[];
};

export const LANGUAGES: Language[] = [
  {
    name: 'slang',
    ext: SlangFileSuffixes.Slang,
    cmdRunFile: [`${path.join(SlangPaths.BinDir, SlangBuildConfigs.Release, 'slang')}`, 'run'],
    cmdGetVersion: [`${path.join(SlangPaths.BinDir, SlangBuildConfigs.Release, 'slang')}`, '--version'],
  },
  {
    name: 'javascript',
    ext: '.js',
    cmdRunFile: ['node', '--jitless', '--noexpose_wasm', '--no-warnings'], // --no-warnings is a workaround for module syntax parsing warnings (MODULE_TYPELESS_PACKAGE_JSON)
    cmdGetVersion: ['node', '--version'],
  },
  {
    name: 'python',
    ext: '.py',
    cmdRunFile: ['py'],
    cmdGetVersion: ['py', '--version'],
  },
];

export const BENCHMARKS: Benchmark[] = [];

/**
 * Define a benchmark
 * @param name - The name of the benchmark (file name, without extension and .bench suffix)
 * @param numRuns - The number of runs to perform for this benchmark
 * @param numWarmupRuns - The number of warmup runs to perform for this benchmark
 * @param expectedOutput - The expected output of the benchmark, each output line as an array element
 */
const defineBenchmark = (name: string, numRuns: number, numWarmupRuns: number, expectedOutput: string[]) => {
  const expectedOutStr = expectedOutput.length > 0 ? expectedOutput.join('\n') + '\n' : '';
  const regex = new RegExp(expectedOutStr + 'elapsed: (\\d+.\\d+)', 'm');
  BENCHMARKS.push({ name, numRuns, numWarmupRuns, regex });
};

defineBenchmark('bfs', 20, 20, ['1533644', '936718']);
defineBenchmark('dhrystone', 20, 20, []);
defineBenchmark('dp', 20, 20, ['215374', '260586897262600']);
defineBenchmark('fib', 20, 20, ['832040', '832040', '832040', '832040', '832040']);
defineBenchmark('list', 20, 20, ['499999500000']);
defineBenchmark('parse', 20, 20, ['168539636', '97529391']);
defineBenchmark('string', 20, 20, []);
defineBenchmark('zoo', 20, 20, ['1800000', '1800000', '1800000', '1800000', '1800000']);

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
 * Retrieves the version of an installed language, which signifies that the language is actually installed.
 * @param lang - The language to get the installed version from
 * @returns A promise which resolves to the version string of the language, or null if the interpreter is not installed / version retrieval failed.
 */
export const runGetVersion = async (lang: Language): Promise<string | null> => {
  let { output: interpreterVersion } = await runProcess(lang.cmdGetVersion.join(' '));
  if (!interpreterVersion) {
    error(`No interpreter fond for language '${lang.name}'.`, `Command '${lang.cmdGetVersion.join(' ')}' failed.`);
    return null;
  }

  if (isLangSlang(lang)) {
    interpreterVersion += ' @ ' + (await gitStatus()).hash;
  }

  interpreterVersion = normalizeLineEndings(interpreterVersion).replace(/\n/g, '');
  return interpreterVersion;
};

/**
 * Runs a single benchmark and returns the result of the run. Prints lang, name and progress to stdout
 * @param lang - The language which executes this run.
 * @param bench - The benchmark to execute.
 * @param benchNamePadding - A right-padding to print after the bench-name is printed.
 * @param langNamePadding  - A right-padding to print after the lang-name is printed.
 * @returns A promise which resolves to either the result, or null, if the run failed.
 */
export const runBenchmark = async (
  lang: Language,
  bench: Benchmark,
  benchNamePadding: number = bench.name.length + 1,
  langNamePadding: number = lang.name.length + 1,
): Promise<(BenchmarkRun & { raw: number[] }) | null> => {
  const { ext, name, cmdRunFile } = lang;
  const filePath = path.join(SlangPaths.BenchDir, bench.name + BENCH_PRE_SUFFIX + ext);
  const runCommand = cmdRunFile.join(' ') + ' ' + filePath;

  const times: number[] = [];

  // Print benchmark header
  const [, , multilineHeader, multilineHeaderStyle] = LOG_CONFIG['info'];
  process.stdout.write(multilineHeaderStyle(multilineHeader));
  process.stdout.write(`  ${chalk.bold(bench.name)} `);
  process.stdout.write(' '.repeat(benchNamePadding - bench.name.length + 1));
  process.stdout.write(`${chalk.italic(name == 'slang' ? chalk.magenta(name) : name)} `);
  process.stdout.write(' '.repeat(langNamePadding - name.length + 1));

  if (!existsSync(filePath)) {
    process.stdout.write(` (file ${filePath} not found)\n`);
    return null;
  }

  // Create a progress spinner
  const SPINNER = '░▒▓';
  const [PENDING, COMPLETED] = '░█';
  const cursor = (show: boolean) => process.stdout.write(show ? '\x1b[?25h' : '\x1b[?25l');
  let i = 0;

  const spinner = setInterval(() => {
    cursor(false); // Hide cursor
    const char = SPINNER.charAt(Math.floor(Date.now() / 80) % SPINNER.length);

    process.stdout.write('\x1b[s'); // Save cursor position

    const progCurrent = char;
    const progDone = COMPLETED.repeat(i);
    const progRemain = PENDING.repeat(bench.numRuns - i);

    const progress = chalk.green(progDone) + progCurrent + chalk.red(progRemain);

    const current = i.toString().padStart(bench.numRuns.toString().length, ' ');
    process.stdout.write(` ${progress} ${current}/${bench.numRuns}`);

    process.stdout.write('\x1b[u'); // Restore cursor position
  }, 100);

  const stopSpinner = () => {
    clearInterval(spinner);
    cursor(true); // Show cursor
  };

  // Warmup
  for (let j = 0; j < bench.numWarmupRuns; j++) await runProcess(runCommand, '', undefined, false, true);

  // Run benchmark
  for (; i < bench.numRuns; i++) {
    const { output } = await runProcess(runCommand, '', undefined, false, true);

    if (!output) {
      stopSpinner();
      process.stdout.write('\n');
      warn(`${lang} running ${bench.name} benchmark failed`, `Received no output`);
      times.push(Infinity);
      break;
    }

    const match = normalizeLineEndings(output).trim().match(bench.regex);
    if (!match || !match[1] /* elapsed time */) {
      stopSpinner();
      process.stdout.write('\n');
      warn(`${lang} output for ${bench.name} does not match expected output`, output);
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

  return {
    best,
    avg,
    worst,
    score,
    sd: standardDev,
    raw: times,
    date: new Date(),
  };
};

/**
 * Prints a benchmark result with comparison. If the [lang] is slang, the result will be
 * compared to the last run of this benchmark on this processor (looked up in [prevResults]).
 * For any other language, the result is compared to the slang result of the current benchmark
 * run (looked up in [currentResults])
 * @param lang - The language of this benchmark result.
 * @param bench - The benchmark which generated this result.
 * @param processorName - The name of the processor which created this result.
 * @param result - The result of the run
 * @param currentResults - All results of the current run - This is used if any other language other
 * than slang is printing results - this should contain the result of the slang run, because slang always runs first.
 * The slang result will form the basis for the comparison.
 * @param prevResults - All previous results, loaded from a serialized file. This is used if slang is printing results -
 * the last added slang entry for this benchmark run will be used for the compaison.
 */
const printBenchmarkResultWithComparison = (
  lang: Language,
  bench: Benchmark,
  processorName: string,
  result: BenchmarkRun,
  currentResults: BenchmarkResult[],
  prevResults: BenchmarkResult[],
): void => {
  let comparison;

  const makeComparison = (dividend: number, divisor: number, suffix: string) => {
    // Calculate comparison
    const ratio = (100 / dividend) * divisor;
    let cmp = ratio.toFixed(2) + suffix;

    if (ratio > 100) {
      cmp = chalk.green(cmp);
    }
    if (ratio < 100) {
      cmp = chalk.red(cmp);
    }

    return cmp;
  };

  if (isLangSlang(lang)) {
    // If we're running a slang benchmark, compare to a previous slang result
    const prevResult = findResult(prevResults, bench.name, 'slang', processorName, false);
    if (prevResult) {
      comparison = makeComparison(result.avg, prevResult.avg, '% relative to baseline');
    } else {
      comparison = chalk.gray('no baseline for this benchmark on this cpu found');
    }
  } else {
    // If we're running a non-slang benchmark, compare to the current slang result. Slang always runs first.
    const slangResult = findResult(currentResults, bench.name, 'slang', processorName);
    if (slangResult) {
      comparison = makeComparison(slangResult.avg, result.avg, '%');
    } else {
      comparison = chalk.gray('no slang result for this benchmark on this cpu found');
    }
  }

  // Emphasize standard deviation if it's high
  const avgStr = 'avg=' + chalk.bold(result.avg.toFixed(3)) + 's';
  const bestStr = 'best=' + result.best.toFixed(3) + 's';
  const worstStr = 'worst=' + result.worst.toFixed(3) + 's';
  const sdStr = 'sd=' + (result.sd > 0.05 ? chalk.yellow(result.sd.toFixed(4)) : result.sd.toFixed(4));

  process.stdout.write('\u001b[K'); // Clear from cursor to end of line
  process.stdout.write(` ${avgStr} ${bestStr} ${worstStr} ${sdStr} ${comparison}\n`);
};

/**
 * Prints a benchmark result.
 * @param result - The result of the run
 */
export const printBenchmarkResult = (result: BenchmarkRun): void => {
  // Emphasize standard deviation if it's high
  const avgStr = 'avg=' + chalk.bold(result.avg.toFixed(3)) + 's';
  const bestStr = 'best=' + result.best.toFixed(3) + 's';
  const worstStr = 'worst=' + result.worst.toFixed(3) + 's';
  const sdStr = 'sd=' + (result.sd > 0.05 ? chalk.yellow(result.sd.toFixed(4)) : result.sd.toFixed(4));

  process.stdout.write('\u001b[K'); // Clear from cursor to end of line
  process.stdout.write(` ${avgStr} ${bestStr} ${worstStr} ${sdStr}\n`);
};

/**
 * Runs all benchmarks and writes results to a log file
 * @param langPattern - Pattern to match languages (regex)
 */
export const runBenchmarks = async (langPattern?: string) => {
  const benchLogFile = path.join(SlangPaths.BenchDir, BENCH_LOG_FILE);
  const prevResults = JSON.parse(await readFile(benchLogFile));
  const results: BenchmarkResult[] = [];

  const processorName = await getProcessorName();
  const actualLanguages = langPattern ? LANGUAGES.filter(l => l.name.match(langPattern)) : LANGUAGES;
  const longestBenchmarkName = Math.max(...BENCHMARKS.map(b => b.name.length));
  const longestLanguageName = Math.max(...actualLanguages.map(l => l.name.length));

  for (const language of actualLanguages) {
    const { name } = language;

    const version = await runGetVersion(language);
    if (!version) {
      warn(`Failed to get version for language '${language.name}'. Skipping benches for it.`);
      continue;
    }

    for (const benchmark of BENCHMARKS) {
      const result = await runBenchmark(language, benchmark, longestBenchmarkName, longestLanguageName);
      if (!result) {
        continue;
      }

      printBenchmarkResultWithComparison(language, benchmark, processorName, result, results, prevResults);

      results.push({
        name: benchmark.name,
        cpu: processorName,
        lang: name,
        v: version,
        // Don't spread the `result` into here, since it also contains the raw run-times from the bench run - we don't want those in the result json.
        avg: result.avg,
        best: result.best,
        date: result.date,
        score: result.score,
        sd: result.sd,
        worst: result.worst,
      });
    }
  }

  // Write results to log file
  info(`Writing results.`, `to ${benchLogFile}`);
  await createOrAppendJsonFile(benchLogFile, results);

  ok(`All benchmarks done.`);
};

/**
 * Determines whether a lanugage is the slang language.
 * @param lang - Input language
 * @returns True, if the provided language is "slang"
 */
const isLangSlang = (lang: Language) => lang.name === 'slang';

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
// const SPINNER = '⣾⣽⣻⢿⡿⣟⣯⣷';
// const [PENDING, COMPLETED] = '▰▰';
// const [PENDING, COMPLETED] = '▬▮';
// const [PENDING, COMPLETED] = '⬡⬢';
// const [PENDING, COMPLETED] = '▢▣';
// const [PENDING, COMPLETED] = '▢▣';
