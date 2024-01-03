import path from 'node:path';
import { BENCH_LOG_FILE, SLANG_BENCH_DIR, SLANG_BENCH_SUFFIX } from './config.js';
import {
  createOrAppendJsonFile,
  exitWithError,
  extractCommentMetadata,
  findFiles,
  info,
  runProcess,
  runSlangFile,
} from './utils.js';

/**
 * Finds all benchmarks in the slang bench directory
 * @returns {string[]} - Array of benchmark file paths
 */
const findBenchmarks = async () => {
  const benchmarks = await findFiles(SLANG_BENCH_DIR, SLANG_BENCH_SUFFIX);

  if (benchmarks.length === 0)
    exitWithError(`No benchmarks found in ${SLANG_BENCH_DIR} with suffix ${SLANG_BENCH_SUFFIX}`);

  info(`Found ${benchmarks.length} slang benchmarks`);

  return benchmarks;
};

/**
 * Get git status of the current repo (commit hash, date, and message)
 * @returns {Promise<{date: string, hash: string, message: string}>} - Promise that resolves to git status
 */
const gitStatus = async () => {
  const formats = ['%ci', '%H', '%s'];
  const cmd = `git log -1 --pretty=format:`;
  info(`Getting git status`, `Command: "${cmd}" and formats "${formats.join(', ')}"`);
  const [date, hash, message] = await Promise.all(
    formats.map(f => runProcess(cmd + f, `Getting git log faied`)),
  );
  return { date, hash, message };
};

/**
 * Get the name of the processor
 * @returns {Promise<string>} - Promise that resolves to the processor name
 */
const getProcessorName = async () => {
  const cmd = 'wmic cpu get name';
  info(`Getting processor name`, `Command: "${cmd}"`);
  const labelAndName = await runProcess(cmd, `Getting processor name failed`);
  return labelAndName.split('\r\n')[1].trim();
};

/**
 * Get a benchmark result factory function for a given benchmark metadata
 * @param {ReturnType<extractCommentMetadata>} metadata - Metadata extracted from benchmark file
 * @returns {(output: string[]) => object} - Benchmark result factory function
 */
const getBenchmarkFactory = metadata => {
  if (metadata[0].type === 'LatencyBenchmark') {
    const [benchmarkTypeHeader, expectedValueHeader, valueHeader, durationInSecsHeader] = metadata;
    const isValid =
      benchmarkTypeHeader.type === 'LatencyBenchmark' &&
      expectedValueHeader.type === 'ExpectedValue' &&
      valueHeader.type === 'Value' &&
      durationInSecsHeader.type === 'DurationInSecs';

    if (!isValid) {
      exitWithError('LatencyBenchmark metadata is invalid.', `Metadata: ${metadata.join('\n')}`);
    }

    const latencyBenchmarkFactory = output => {
      if (output.length !== 2) {
        exitWithError(
          `LatencyBenchmark output is invalid, expected 2 lines, got ${output.length}.`,
          `Output: ${output.join('\n')}`,
        );
      }

      const [value, durationInSecs] = output;

      if (value !== expectedValueHeader.value) {
        exitWithError(
          `LatencyBenchmark value is invalid, expected ${expectedValueHeader.value}, got ${value}.`,
          `Output: ${output.join('\n')}`,
        );
      }

      return {
        benchmarkType: 'LatencyBenchmark',
        name: benchmarkTypeHeader.value,
        expectedValue: expectedValueHeader.value,
        value,
        durationInSecs,
      };
    };
    return latencyBenchmarkFactory;
  } else if (metadata[0].type === 'ThroughputBenchmark') {
    const [benchmarkTypeHeader, throughputHeader, valueHeader, durationInSecsHeader] = metadata;
    const isValid =
      benchmarkTypeHeader.type === 'ThroughputBenchmark' &&
      throughputHeader.type === 'Throughput' &&
      valueHeader.type === 'Value' &&
      durationInSecsHeader.type === 'DurationInSecs';

    if (!isValid) {
      exitWithError('ThroughputBenchmark metadata is invalid.', `Metadata: ${metadata.join('\n')}`);
    }

    const throughputBenchmarkFactory = output => {
      if (output.length !== 3) {
        exitWithError(
          `ThroughputBenchmark output is invalid, expected 3 lines, got ${output.length}.`,
          `Output: ${output.join('\n')}`,
        );
      }

      const [throughput, value, durationInSecs] = output;

      return {
        benchmarkType: 'ThroughputBenchmark',
        name: benchmarkTypeHeader.value,
        throughput,
        value,
        durationInSecs,
      };
    };
    return throughputBenchmarkFactory;
  }

  exitWithError('Unkown benchmark type.', `Metadata: ${metadata.join('\n')}`);
};

/**
 * Runs all benchmarks and writes results to a log file
 * @param {string[]} configs - Array of build configs to run benchmarks for
 */
export const runBenchmarks = async configs => {
  const benchmarksPaths = await findBenchmarks();

  const benchmarks = [];
  for (const filePath of benchmarksPaths) {
    const commentMetadata = await extractCommentMetadata(filePath);
    const benchmarkFactory = getBenchmarkFactory(commentMetadata);
    benchmarks.push({
      filePath,
      benchmarkFactory,
    });
  }

  const results = [];

  const date = new Date();
  const commit = await gitStatus();
  const processorName = await getProcessorName();

  for (const benchmark of benchmarks) {
    info('Running benchmark', benchmark.filePath);

    for (const config of configs) {
      const { output, exitCode } = await runSlangFile(benchmark.filePath, config);

      if (exitCode !== 0) {
        exitWithError(
          `Benchmark failed with exit code ${exitCode}.`,
          `Output: ${output.join('\n')}`,
        );
      }

      const benchmarkResult = benchmark.benchmarkFactory(output);
      results.push({
        date,
        commit,
        processorName,
        config,
        benchmark: benchmarkResult,
      });
    }
  }

  info(`Done.`, `Writing results to ${path.join(SLANG_BENCH_DIR, BENCH_LOG_FILE)}`);

  const benchLogFile = path.join(SLANG_BENCH_DIR, BENCH_LOG_FILE);
  await createOrAppendJsonFile(benchLogFile, results, (existingResults, newResults) => {
    return [...existingResults, ...newResults];
  });
};
