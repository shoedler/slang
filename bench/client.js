const colors = [
  "rgb(255, 99, 132)",
  "rgb(54, 162, 235)",
  "rgb(255, 205, 86)",
  "rgb(75, 192, 192)",
  "rgb(153, 102, 255)",
  "rgb(255, 159, 64)",
];

document.addEventListener("DOMContentLoaded", () => {
  fetch("http://localhost:8080/results")
    .then((response) => response.json())
    .then((data) => {
      findConfigs(data).forEach((config) => {
        const h2 = document.createElement("h2");
        h2.textContent = config + " runs";
        document.getElementById("charts").appendChild(h2);

        const canvas = document.createElement("canvas");
        canvas.id = "chart-" + config;
        document.getElementById("charts").appendChild(canvas);

        const { benchmarkData, benchmarkDates: labels } = mapBenchmarkData(
          data,
          config
        );
        const datasets = createDatasets(benchmarkData);
        const options = createChartOptions();

        new Chart(canvas, {
          type: "line",
          data: {
            labels,
            datasets,
          },
          options,
        });
      });
    });
});

/**
 * Find all configs in the data
 * @param {import("../runner/src/bench").BenchmarkResult[]} data
 * @returns {import("../runner/src/bench").BenchmarkResult['config'][]} configs
 */
const findConfigs = (data) => {
  const configs = new Set();
  data.forEach((entry) => {
    configs.add(entry.config);
  });

  return Array.from(configs).sort((a, b) => {
    // "Release" should be first
    if (a === "Release") {
      return -1;
    }
  });
};

/**
 * Organize the data into a format that is easier to use for chart.js
 * @param {import("../runner/src/bench").BenchmarkResult[]} data
 * @param {import("../runner/src/bench").BenchmarkResult['config']} config
 * @returns {{ benchmarkData: Object, benchmarkDates: Date[] }} organized data
 */
const mapBenchmarkData = (data, config) => {
  const benchmarkData = {};
  const benchmarkDates = [];

  data
    .filter((entry) => entry.config === config)
    .forEach((entry) => {
      const type = entry.benchmark.benchmarkType;
      if (!benchmarkData[type]) {
        benchmarkData[type] = {};
      }

      const name = entry.benchmark.name;
      if (!benchmarkData[type][name]) {
        benchmarkData[type][name] = [];
      }

      // Depending on the benchmark, decide what value is relevant
      let value;
      switch (type) {
        case "LatencyBenchmark":
          value = parseFloat(entry.benchmark.durationInSecs);
          break;
        case "ThroughputBenchmark":
          value = parseInt(entry.benchmark.throughput, 10);
          break;
        default:
          console.error("Unknown benchmark type", entry);
          return;
      }

      const date = new Date(entry.date);
      benchmarkDates.push(date);
      benchmarkData[type][name].push({
        y: value,
        x: date,
        label: `
Commit-Hash: ${entry.commit.hash}

Commit-Message:
${addLinebreaks(entry.commit.message, 50)}

Note: 
${addLinebreaks(entry.note || "-", 50)}
              `,
      });
    });

  return { benchmarkData, benchmarkDates };
};

/**
 * Create datasets for the chart
 * @param {Object} benchmarkData
 * @returns {Array} datasets
 */
const createDatasets = (benchmarkData) => {
  const datasets = [];
  Object.keys(benchmarkData).forEach((type, index) => {
    Object.keys(benchmarkData[type]).forEach((name) => {
      const dataset = {
        label: `${type} - (${name})`,
        data: benchmarkData[type][name],
        borderColor: colors[index % colors.length] || "rgb(255, 99, 132)",
        borderWidth: 4,
        pointRadius: 6,

        tension: 0,
      };

      datasets.push(dataset);
    });
  });
  return datasets;
};

/**
 * Add linebreaks to a string
 * @param {String} text
 * @param {Number} maxLen
 * @returns {String} text
 */
const addLinebreaks = (text, maxLen) => {
  const words = text.split(" ");
  let line = "";
  let lines = [];
  words.forEach((word) => {
    if (line.length + word.length > maxLen) {
      lines.push(line);
      line = "";
    }
    line += word + " ";
  });
  lines.push(line);
  return lines.join("\n");
};

/**
 * Create options for the chart
 * @returns {Object} options
 */
const createChartOptions = () => {
  const FONT_SIZE_SCALES = 16;
  const FONT_SIZE_PLUGINS = 14;
  const LAYOUT_PADDING = 30;
  const LEGEND_PADDING = 20;
  const GRID_LINE_WIDTH = 3;

  const layout = {
    padding: {
      top: LAYOUT_PADDING,
      right: LAYOUT_PADDING,
      bottom: LAYOUT_PADDING,
      left: LAYOUT_PADDING,
    },
  };

  const scales = {
    y: {
      beginAtZero: true,
      ticks: {
        font: {
          size: FONT_SIZE_SCALES,
        },
      },
      grid: {
        lineWidth: GRID_LINE_WIDTH,
      },
    },
    x: {
      type: "time",
      ticks: {
        font: {
          size: FONT_SIZE_SCALES,
        },
      },
      grid: {
        lineWidth: GRID_LINE_WIDTH,
      },
      time: {
        unit: "day",
        adapters: {
          date: {
            locale: "deCH",
          },
        },
      },
    },
  };

  const plugins = {
    zoom: {
      pan: {
        enabled: true,
        mode: "x",
      },
      zoom: {
        wheel: {
          enabled: true,
        },
        mode: "x",
      },
    },
    legend: {
      labels: {
        font: {
          size: FONT_SIZE_PLUGINS,
          padding: LEGEND_PADDING,
        },
      },
    },
    tooltip: {
      bodyFont: {
        size: FONT_SIZE_PLUGINS,
      },
      callbacks: {
        beforeBody: (context) => {
          const point = context[0].raw;
          if (point.label) {
            return point.label;
          }
          return "";
        },
      },
    },
  };

  return { layout, scales, plugins };
};
