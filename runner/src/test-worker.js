import { parentPort } from 'node:worker_threads';
import { runSingleTest, WORK_TYPE_RUN_TEST, WORKER_MESSAGE_TYPE_RESULT } from './test.js';

parentPort.on('message', async ({ type, test, config }) => {
  if (type === WORK_TYPE_RUN_TEST) {
    const result = await runSingleTest(test, config, null);
    parentPort.postMessage({ type: WORKER_MESSAGE_TYPE_RESULT, data: result });
  }
});
