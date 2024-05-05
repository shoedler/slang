const { spawn } = require("node:child_process");
const fs = require("fs");
const path = require("path");

const MSBUILD_EXE =
  '"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\msbuild.exe"';
const SLANG_PROJ_DIR = "C:\\Projects\\slang-opcode-width";

const BUILD_CONFIG = "Release";
const SLANG_BIN_DIR = SLANG_PROJ_DIR + "\\x64";
const SLANG_BENCH_DIR = SLANG_PROJ_DIR + "\\bench";

const SLANG_BENCH_SUFFIX = ".bench.sl";
const BENCH_LOG_FILE = "bench-log.csv";

const SLANG_BUILD_COMMAND = (config) =>
  `cd ${SLANG_PROJ_DIR} && ${MSBUILD_EXE} Slang.sln /p:Configuration=${config}`;

const findBenchmarks = () => {
  const benchmarks = fs
    .readdirSync(SLANG_BENCH_DIR)
    .filter((f) => f.endsWith(SLANG_BENCH_SUFFIX))
    .map((f) => path.join(SLANG_BENCH_DIR, f));

  if (benchmarks.length === 0)
    throw new Error(`No benchmarks found in ${SLANG_BENCH_DIR} with suffix ${SLANG_BENCH_SUFFIX}`);

  console.log(`Found ${benchmarks.length} slang benchmarks:`);
  benchmarks.forEach((b) => console.log(`  ${b}`));

  return benchmarks;
};

const runProcess = (cmd, errorMessage) => {
  const child = spawn(cmd, { shell: true });
  return new Promise((resolve) => {
    let output = "";
    let error = "";

    child.stdout.on("data", (data) => (output += data.toString()));
    child.stderr.on("data", (data) => (error += data.toString()));

    child.on("exit", (code) => {
      if (code === 0 && error === "") {
        resolve(output);
      } else {
        if (error !== "") {
          console.error("Failed due to stderr:");
          console.error(error);
        }
        if (code !== 0) {
          console.error(`Failed due to exit code ${code}`);
        }
        throw new Error(errorMessage);
      }
    });
  });
};

const buildSlang = async (config) => {
  const cmd = SLANG_BUILD_COMMAND(config);
  console.log(`Building slang ${config}`, `(Command: "${cmd}")`);
  return await runProcess(cmd, "Exiting. Build failed");
};

const getProcessorName = async () => {
  const cmd = "wmic cpu get name";
  console.log(`Getting processor name`, `(Command: "${cmd}")`);
  const labelAndName = await runProcess(cmd, `Exiting. Getting processor name failed`);
  return labelAndName.split("\r\n")[1].trim();
};

const getOpcodeSize = () => {
  // It's a #define in common.h named #define OPC_T uint<n>_t
  // We just extract the type after the macro name
  const commonH = path.join(SLANG_PROJ_DIR, "common.h");
  const commonHContent = fs.readFileSync(commonH, "utf8");
  const match = commonHContent.match(/#define\s+OPC_T\s+([^\s]+)/);
  if (!match) throw new Error(`Failed to find opcode size in ${commonH}`);
  return match[1];
};

const run = async () => {
  const benchmarks = findBenchmarks();
  const opcodeSize = getOpcodeSize();
  const processorName = await getProcessorName();

  await buildSlang(BUILD_CONFIG);

  let benchResults = "";

  const addResult = (date, name, result) => {
    benchResults += `${date.toISOString()},${processorName},${opcodeSize},${name},${result}\n`;
  };

  for (const benchmark of benchmarks) {
    const date = new Date();

    for (let i = 0; i < 10; i++) {
      console.log(`  ${benchmark}, run ${i + 1}/10`);

      const cmd = `${path.join(SLANG_BIN_DIR, BUILD_CONFIG, "Slang.exe")} run ${benchmark}`;
      const out = await runProcess(cmd, "Exiting. Running benchmark failed");
      const result = out.split("\r\n").join(",");

      const benchName = path.basename(benchmark, SLANG_BENCH_SUFFIX);
      addResult(date, benchName, result);
    }
  }

  console.log(`Done. Writing results to ${path.join(SLANG_BENCH_DIR, BENCH_LOG_FILE)}`);

  const benchLog = path.join(SLANG_BENCH_DIR, BENCH_LOG_FILE);
  const benchLogExists = fs.existsSync(benchLog);
  fs.writeFileSync(benchLog, benchResults, {
    flag: benchLogExists ? "a" : "w",
  });
};

run();
