import path from 'node:path';
import { SlangBuildConfigs, SlangFileSuffixes, SlangPaths } from './config.ts';
import { abort, addAbortHandler, buildSlangConfig, findFiles, info } from './utils.ts';

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
  const binaries = await findBinaries();

  for (const binary of binaries) {
    info(`Benching profile '${binary}'`);
  }
};
