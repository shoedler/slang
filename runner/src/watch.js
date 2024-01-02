import fs from "node:fs";
import { exitWithError, info, warn } from "./utils.js";

/**
 * Watch for changes according to the given trigger in path and run the given action when triggered.
 * Handles debouncing and aborting previous runs.
 * @param {string} path - Path to watch, can be a file or directory
 * @param {fs.watchOptions} watchOptions - Options to pass to fs.watch
 * @param {(filename: string) => boolean} trigger - Function that determines if the action should be triggered, based on a file that changed
 * @param {(signal: AbortSignal) => Promise<void>} action - Action to perform when a file changes. Must return a promise that resolves when the action is complete.
 */
export const watch = (path, watchOptions, trigger, action) => {
  let controller = undefined;
  let timeout = undefined;

  info(
    "Watching for changes",
    `Path: ${path}, Trigger: ${trigger.toString().replace(/\=\>\s+/, "=> ")}`
  );
  info("Stdout and stderr might not be in order");
  info("Exit with SIGINT", "Ctrl+C");

  // Watch for changes
  fs.watch(path, watchOptions, async (_, filename) => {
    if (!filename) {
      return;
    }

    if (trigger(filename)) {
      // Debounce the event
      if (timeout) {
        clearTimeout(timeout);
      }

      timeout = setTimeout(async () => {
        // Abort the previous run if it is still running
        if (controller) {
          controller.abort();
        }
        controller = new AbortController();

        try {
          info("Change detected", filename);
          await action(controller.signal);
          info("Waiting for changes...");
        } catch (err) {
          if (err.name === "AbortError") {
            warn("Aborted current run.");
          } else {
            controller.abort();
            exitWithError("Error while running watch-action", err);
          }
        }
      }, 200);
    }
  });
};
