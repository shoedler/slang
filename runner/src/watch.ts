import fs, { PathLike } from 'node:fs';
import { clearTimeout, setTimeout } from 'node:timers';
import { LOCALE } from './config.ts';
import { abort, debug, info, warn } from './utils.ts';

/**
 * Watch for changes according to the given trigger in path and run the given action when triggered.
 * Handles debouncing and aborting previous runs.
 * @param path - Path to watch, can be a file or directory
 * @param watchOptions - Options to pass to fs.watch
 * @param trigger - Function that determines if the action should be triggered, based on a file that changed
 * @param action - Action to perform when a file changes. Must return a promise that resolves when the action is complete.
 *                 First run is indicated by `triggerFilename` being undefined.
 */
export const watch = (
  path: PathLike,
  watchOptions: fs.WatchOptions,
  trigger: (filename: string) => boolean,
  action: (signal: AbortSignal, triggerFilename: string | undefined) => Promise<void>,
) => {
  let controller: AbortController | undefined = undefined;
  let timeout: NodeJS.Timeout | undefined = undefined;

  // Wrapper function to handle aborting previous runs. Also handy to run this initially.
  const abortableAction = async (triggerFilename: string | undefined) => {
    // Abort the previous run if it is still running
    if (controller) {
      controller.abort();
    }
    controller = new AbortController();

    try {
      const now = new Date();
      info('Change detected, running action...', triggerFilename);
      const start = performance.now();
      await action(controller.signal, triggerFilename);
      const duration = performance.now() - start;
      debug(
        `Last run was on ${now.toLocaleDateString(LOCALE) + ' at ' + now.toLocaleTimeString(LOCALE)}`,
        `Took ${duration.toFixed(2)}ms`,
      );
      info('Waiting for changes...', `Path: ${path}, Trigger: ${trigger.toString().replace(/\=\>\s+/, '=> ')}`);
      info('Exit with SIGINT', 'Ctrl+C');
      warn('Stdout and stderr might not be in order');
    } catch (err) {
      const error = err as NodeJS.ErrnoException;
      if (error.name === 'AbortError') {
        warn('Aborted current run.');
      } else {
        controller.abort();
        abort('Error while running watch-action', error.toString());
      }
    }
  };

  // Run the initial action
  abortableAction(undefined);

  // Watch for changes
  fs.watch(path, watchOptions, (_, filename) => {
    if (!filename) {
      return;
    }

    if (trigger(filename)) {
      if (timeout) {
        clearTimeout(timeout);
      }

      timeout = setTimeout(async () => await abortableAction(filename), 200);
    }
  });
};
