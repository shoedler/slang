import { parentPort } from 'node:worker_threads';
import { runSingleTest, WorkerMessageArgs, WorkerMessageTypes, WorkMessageArgs, WorkMessageTypes } from './test.ts';

if (!parentPort) {
  throw new Error('This module must be run in a worker thread');
}

const pp = parentPort!;

pp.on('message', async arg => {
  const { type } = arg;
  if (type === WorkMessageTypes.RunTest) {
    const { test, config, runFlags, updateFile } = <WorkMessageArgs[WorkMessageTypes.RunTest]>arg;
    const result = await runSingleTest(test, config, runFlags, updateFile, null);
    pp.postMessage(<WorkerMessageArgs[WorkerMessageTypes.Result]>{ type: WorkerMessageTypes.Result, result });
  }
});
