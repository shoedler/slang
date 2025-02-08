import path from 'node:path';
import process from 'node:process';
import {
  BenchmarkResult,
  BENCHMARKS,
  Language,
  LANGUAGES,
  printBenchmarkResult,
  runBenchmark,
  runGetVersion,
} from './bench.ts';
import { LOCALE, PGO_BENCH_LOG_FILE, SlangBuildConfigs, SlangFileSuffixes, SlangPaths } from './config.ts';
import {
  abort,
  addAbortHandler,
  buildSlangConfig,
  createOrAppendJsonFile,
  findFiles,
  getProcessorName,
  info,
  ok,
  separator,
  warn,
} from './utils.ts';

/**
 * Finds all profiles in the slang profile directory
 * @returns Array of profile file paths
 */
const findProfiles = async () => {
  const profiles = await findFiles(SlangPaths.ProfileDir, SlangFileSuffixes.Profile);

  if (profiles.length === 0) {
    abort(`No profiles found in '${SlangPaths.ProfileDir}' with suffix '${SlangFileSuffixes.Profile}'`);
  }

  const allProfilesString = profiles.join('\n');
  info(`Found ${profiles.length} profiles in '${SlangPaths.ProfileDir}'`, '\n' + allProfilesString);

  return profiles;
};

/**
 * Finds all built slang binaries in the profile directory
 * @returns Array of profiled slang binary file paths
 */
const findBinaries = async () => {
  const binaries = await findFiles(SlangPaths.ProfileDir, SlangFileSuffixes.Binary);

  if (binaries.length === 0) {
    abort(
      `No binaries found in '${SlangPaths.ProfileDir}' with suffix '${SlangFileSuffixes.Binary}'. Need to build the binaries first.`,
    );
  }

  const allBinariesString = binaries.join('\n');
  info(`Found ${binaries.length} binaries in '${SlangPaths.ProfileDir}'`, '\n' + allBinariesString);

  return binaries;
};

const stripSuffix = (str: string, suffix: string) => str.slice(0, str.length - suffix.length);

const backupProfile = async () => {
  try {
    await Deno.rename(SlangPaths.ProfileFile, SlangPaths.ProfileFile + '.bak');
    addAbortHandler(restoreProfile); // Add abort handler to restore the profile AFTER creating the backup.
  } catch {
    abort(`No existing profile file found at '${SlangPaths.ProfileFile}'`);
  }
};

const restoreProfile = () => {
  Deno.renameSync(SlangPaths.ProfileFile + '.bak', SlangPaths.ProfileFile);
};

export const runPgoBuildProfiles = async () => {
  await backupProfile();
  try {
    const profiles = await findProfiles();

    for (const profile of profiles) {
      info(`Making profile '${profile}'`);
      // Copy the profile to the profile file (overwriting or creating it)
      await Deno.copyFile(profile, SlangPaths.ProfileFile);
      // Build with PGO
      await buildSlangConfig(SlangBuildConfigs.ReleaseProfiled, null, true);
      // Copy the buillt binary to the profile dir, <profileName>.slang.exe
      const binPath = path.join(SlangPaths.BinDir, SlangBuildConfigs.Release, 'slang.exe');
      const targetPath = stripSuffix(profile, SlangFileSuffixes.Profile) + SlangFileSuffixes.Binary;
      await Deno.copyFile(binPath, targetPath);
    }

    // deno-lint-ignore no-explicit-any
  } catch (e: any) {
    abort('Failed to build profiles', e.toString());
  }
  restoreProfile();
};

const pgoBenchLogFilePath = path.join(SlangPaths.ProfileDir, PGO_BENCH_LOG_FILE);
const makePgoBenchRawFilePath = (name: string, bench: string) =>
  path.join(SlangPaths.ProfileDir, 'pgo-raw-' + name + '-' + bench + '.json');

export const runPgoBenchProfiles = async (numRuns = 10) => {
  info(`Running PGO bench-suite ${numRuns} times`);
  separator();

  const binaries = (await findBinaries()).map(binary => {
    const tppName = stripSuffix(path.basename(binary), SlangFileSuffixes.Binary);
    return {
      binPath: binary,
      tppName,
      name: 'slang(TPP=' + tppName + ')',
    };
  });

  // Add a normal release build to the binaries
  info('Building and adding release build to the binaries');
  await buildSlangConfig(SlangBuildConfigs.Release, null, true);
  binaries.push({
    binPath: path.join(SlangPaths.BinDir, SlangBuildConfigs.Release, 'slang'),
    tppName: 'control',
    name: 'slang(release)',
  });

  const slangDef = LANGUAGES.find(lang => lang.name === 'slang')!;
  const processorName = await getProcessorName();
  const longestBenchmarkName = Math.max(...BENCHMARKS.map(b => b.name.length));
  const longestLanguageName = Math.max(...binaries.map(l => l.name.length));

  const start = process.hrtime.bigint();
  const startDate = new Date();

  for (let i = 0; i < numRuns; i++) {
    // Shuffle to distribute errors at start/end of run or hot/cold CPU
    binaries
      .map(value => ({ value, sort: Math.random() }))
      .sort((a, b) => a.sort - b.sort)
      .map(({ value }) => value);

    for (const binary of binaries) {
      info(`Benching profile '${binary.binPath}'`);
      const results: BenchmarkResult[] = [];

      // Update the lang-defs paths and name to the new binary
      slangDef.name = binary.name;
      slangDef.cmdRunFile = [binary.binPath, 'run'];
      slangDef.cmdGetVersion = [binary.binPath, '--version'];

      // Retrieve version
      const version = await runGetVersion(slangDef);
      if (!version) {
        warn(`Failed to get version with binary '${slangDef.name}'. Skipping benches for it.`);
        continue;
      }

      for (const benchmark of BENCHMARKS) {
        // Run
        benchmark.numRuns = 100;
        const result = await runBenchmark(slangDef, benchmark, longestBenchmarkName, longestLanguageName);
        if (!result) {
          continue;
        }

        printBenchmarkResult(result);

        // Write raw results to log file
        const pgoBenchRawFilePath = makePgoBenchRawFilePath(binary.tppName, benchmark.name);
        await createOrAppendJsonFile(pgoBenchRawFilePath, [result]);

        results.push({
          name: benchmark.name,
          cpu: processorName,
          lang: binary.name,
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

      // Write results to log file
      info(`Took: ${(Number(process.hrtime.bigint() - start) / 1_000_000_000).toFixed(5)}s`);
      info(`Writing results.`, `to ${pgoBenchLogFilePath}`);
      await createOrAppendJsonFile(pgoBenchLogFilePath, results);
    }
    info(
      `Run ${i + 1} completed. Started: ${startDate.toLocaleString(LOCALE)}, ended: ${new Date().toLocaleString(
        LOCALE,
      )}`,
    );
    separator();
  }

  info(`Started: ${startDate.toLocaleString(LOCALE)}, ended: ${new Date().toLocaleString(LOCALE)}`);
  const time = `Took: ${(Number(process.hrtime.bigint() - start) / 1_000_000_000).toFixed(5)}s`;
  ok(`All benchmarks done. ${time}`);
};

const extendedBenchLogFilePath = path.join(SlangPaths.ProfileDir, 'extended-bench-log.json');
const makeExtendedBenchRawFilePath = (name: string, bench: string) =>
  path.join(SlangPaths.ProfileDir, 'extended-bench-raw-' + name + '-' + bench + '.json');

export const runExtendedBench = async (numRuns = 10) => {
  const BEST_PGO_BINARY = 'C:\\Projects\\slang\\profile\\list.slang.exe';

  info(`Running extended bench-suite ${numRuns} times`);
  separator();

  const actualLanguages = JSON.parse(JSON.stringify(LANGUAGES)) as Language[];
  const processorName = await getProcessorName();

  // Add another slang language-definition for the best PGO binary
  const slangLang = LANGUAGES.find(lang => lang.name === 'slang')!;
  slangLang.name = 'slang(PGO-TPP=' + stripSuffix(path.basename(BEST_PGO_BINARY), SlangFileSuffixes.Binary) + ')';
  slangLang.cmdRunFile = [BEST_PGO_BINARY, 'run'];
  slangLang.cmdGetVersion = [BEST_PGO_BINARY, '--version'];
  actualLanguages.push(slangLang);

  const longestBenchmarkName = Math.max(...BENCHMARKS.map(b => b.name.length));
  const longestLanguageName = Math.max(...actualLanguages.map(l => l.name.length));

  const start = process.hrtime.bigint();
  const startDate = new Date();

  for (let i = 0; i < numRuns; i++) {
    // Shuffle to distribute errors at start/end of run or hot/cold CPU
    actualLanguages
      .map(value => ({ value, sort: Math.random() }))
      .sort((a, b) => a.sort - b.sort)
      .map(({ value }) => value);

    for (const lang of actualLanguages) {
      info(`Benching language '${lang.name}'`);
      const results: BenchmarkResult[] = [];

      // Retrieve version
      const version = await runGetVersion(lang);
      if (!version) {
        warn(`Failed to get version for language '${lang.name}'. Skipping benches for it.`);
        continue;
      }

      for (const benchmark of BENCHMARKS) {
        // Run
        benchmark.numRuns = 100;
        const result = await runBenchmark(lang, benchmark, longestBenchmarkName, longestLanguageName);
        if (!result) {
          continue;
        }

        printBenchmarkResult(result);

        // Write raw results to log file
        const extendedBenchRawFilePath = makeExtendedBenchRawFilePath(lang.name, benchmark.name);
        await createOrAppendJsonFile(extendedBenchRawFilePath, [result]);

        results.push({
          name: benchmark.name,
          cpu: processorName,
          lang: lang.name,
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

      // Write results to log file
      info(`Took: ${(Number(process.hrtime.bigint() - start) / 1_000_000_000).toFixed(5)}s`);
      info(`Writing results.`, `to ${extendedBenchLogFilePath}`);
      await createOrAppendJsonFile(extendedBenchLogFilePath, results);
    }
    info(
      `Run ${i + 1} completed. Started: ${startDate.toLocaleString(LOCALE)}, ended: ${new Date().toLocaleString(
        LOCALE,
      )}`,
    );
    separator();
  }

  info(`Started: ${startDate.toLocaleString(LOCALE)}, ended: ${new Date().toLocaleString(LOCALE)}`);
  const time = `Took: ${(Number(process.hrtime.bigint() - start) / 1_000_000_000).toFixed(5)}s`;
  ok(`All benchmarks done. ${time}`);
};
