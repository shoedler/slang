import path from 'node:path';
import process from 'node:process';
import { BenchmarkResult, BENCHMARKS, LANGUAGES, printBenchmarkResult, runBenchmark, runGetVersion } from './bench.ts';
import { PGO_BENCH_LOG_FILE, SlangBuildConfigs, SlangFileSuffixes, SlangPaths } from './config.ts';
import {
  abort,
  addAbortHandler,
  buildSlangConfig,
  createOrAppendJsonFile,
  findFiles,
  getProcessorName,
  info,
  ok,
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

export const runPgoBenchProfiles = async () => {
  const benchLogFile = path.join(SlangPaths.BenchDir, PGO_BENCH_LOG_FILE);
  const results: BenchmarkResult[] = [];

  const binaries = (await findBinaries()).map(binary => {
    return {
      binPath: binary,
      name: 'slang(PgoProfile=' + stripSuffix(path.basename(binary), SlangFileSuffixes.Binary) + ')',
    };
  });

  const slangDef = LANGUAGES.find(lang => lang.name === 'slang')!;
  const processorName = await getProcessorName();
  const longestBenchmarkName = Math.max(...BENCHMARKS.map(b => b.name.length));
  const longestLanguageName = Math.max(...binaries.map(l => l.name.length));

  const start = process.hrtime.bigint();

  for (const binary of binaries) {
    info(`Benching profile '${binary.binPath}'`);

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

      results.push({
        name: benchmark.name,
        cpu: processorName,
        lang: binary.name,
        v: version,
        ...result,
      });
    }

    // Write results to log file
    info(`Writing results.`, `to ${benchLogFile}`);
    await createOrAppendJsonFile(benchLogFile, results);
  }

  const time = `Took: ${(Number(process.hrtime.bigint() - start) / 1_000_000_000).toFixed(5)}s`;
  ok(`All benchmarks done. ${time}`);
};
