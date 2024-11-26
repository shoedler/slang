const colors = {
  slang: "rgb(255, 99, 132)",
  python: "rgb(54, 162, 235)",
  javascript: "rgb(255, 205, 86)",
};

document.addEventListener("DOMContentLoaded", () => {
  const { horizontalHoverLinePlugin } = createChartPlugins();
  Chart.register(horizontalHoverLinePlugin);

  // Use the default font family from the document body
  Chart.defaults.font.family = getComputedStyle(document.body).fontFamily;
  Chart.defaults.color = getComputedStyle(document.body).color;
  Chart.defaults.borderColor = "#303030";

  fetch("http://localhost:5252/results")
    .then((response) => response.json())
    .then((data) => {
      const benchmarkData = mapBenchmarkData(data);
      const chartsContainer = document.getElementById("charts");
      console.log(benchmarkData);

      for (const cpu in benchmarkData) {
        // Section for each CPU
        appendHeading(chartsContainer, "h4", "CPU: " + cpu);

        for (const benchmarkName in benchmarkData[cpu]) {
          const chartId = `chart-${cpu}-${benchmarkName}`;

          // Chart for each benchmark name
          const container = appendContainer(chartsContainer);
          const headingContainer = appendContainer(
            container,
            "heading-container"
          );
          appendHideShowButton(headingContainer, chartId + "-container");
          appendHeading(headingContainer, "h2", benchmarkName);

          // Create the chart
          const chart = appendChart(container, chartId);

          const datasets = [];
          const labels = new Set();
          for (const lang in benchmarkData[cpu][benchmarkName]) {
            const runs = benchmarkData[cpu][benchmarkName][lang];
            const color = colors[lang];
            if (!color) {
              alert("Unknown language in mapped benchmark data: " + lang);
            }

            const dataset = {
              label: `${lang}: ${benchmarkName}`,
              data: runs,
              borderColor: color,
              borderWidth: 3,
              pointRadius: 3,
              pointBackgroundColor: color,
              tension: 0,
            };
            datasets.push(dataset);

            for (const point of runs) {
              labels.add(
                new Date(point.x).toLocaleDateString("de-CH", {
                  year: "2-digit",
                  month: "short",
                  day: "numeric",
                  hour: "2-digit",
                  minute: "2-digit",
                  second: "2-digit",
                })
              );
            }
          }
          // Merge the datasets
          chart.data = {
            labels: Array.from(labels),
            datasets,
          };
          chart.update();
        }
      }
    });
});

/**
 * Append a container to a parent element
 * @param {HTMLElement} parent - parent element
 * @param {string | null} className - class name for the container, if any
 * @returns {HTMLDivElement} the created container
 */
const appendContainer = (parent, className = "container") => {
  const container = document.createElement("div");
  if (className) {
    container.classList.add(className);
  }
  parent.appendChild(container);
  return container;
};

/**
 * Append a heading to a parent element
 * @param {HTMLElement} parent - parent element
 * @param {`h${number}`} type - heading type
 * @param {string} text - heading text
 * @returns {HTMLHeadingElement} the created heading
 */
const appendHeading = (parent, type, text) => {
  const heading = document.createElement(type);
  heading.textContent = text;
  parent.appendChild(heading);
  return heading;
};

/**
 * Append a hide/show button to a parent element
 * @param {HTMLElement} parent - parent element
 * @param {string} hideShowElementId - id of the element to hide/show
 * @returns {HTMLButtonElement} the created button
 */
const appendHideShowButton = (parent, hideShowElementId) => {
  const hideShowButton = document.createElement("button");
  hideShowButton.classList.add("hide-show-button");
  hideShowButton.textContent = "Hide/Show";
  hideShowButton.onclick = () => {
    const element = document.getElementById(hideShowElementId);
    element.style.display = element.style.display === "none" ? "block" : "none";
  };
  parent.appendChild(hideShowButton);
  return hideShowButton;
};

/**
 * Append a chart to a parent element
 * @param {HTMLElement} parent - parent element
 * @param {string} chartId - id of the chart
 * @returns {Chart} the created chart
 * @see https://www.chartjs.org/docs/latest/
 */
const appendChart = (parent, chartId) => {
  // Container
  const canvasContainer = document.createElement("div");
  canvasContainer.classList.add("chart-container");
  canvasContainer.id = chartId + "-container";
  canvasContainer.style.display = "none"; // Hide the container by default

  // Canvas
  const canvas = document.createElement("canvas");
  canvas.id = chartId;
  parent.appendChild(canvasContainer);
  canvasContainer.appendChild(canvas);

  // Chart
  const chart = new Chart(canvas, {
    type: "line",
    data: {},
    options: createChartOptions(),
  });

  return chart;
};

/**
 * Organize the data into a format that is easier to use for chart.js
 * @param {import("../runner/src/bench").BenchmarkResult[]} data
 * @returns {{ benchmarkData: Object }} organized data
 */
const mapBenchmarkData = (data) => {
  const benchmarkData = {};

  data.forEach((entry) => {
    // Organize the data by processor, benchmark type, language, and benchmark name
    const cpu = entry.cpu;
    if (!benchmarkData[cpu]) {
      benchmarkData[cpu] = {};
    }

    const name = entry.name;
    if (!benchmarkData[cpu][name]) {
      benchmarkData[cpu][name] = {};
    }

    const lang = entry.lang;
    if (!benchmarkData[cpu][name][lang]) {
      benchmarkData[cpu][name][lang] = [];
    }

    const { avg, best, worst, score, sd, v, date: rawDate } = entry;

    // Add the date to the x-axis
    const date = new Date(rawDate);

    // Make a label for the tooltip
    let label = "";
    label += "Average: " + avg + "\n";
    label += "Score: " + score + "\n";
    label += "Best: " + best + "\n";
    label += "Worst: " + worst + "\n";
    label += "Standard Deviation: " + sd + "\n";
    label += (lang == "slang" ? "Commit-Hash: " : "Version: ") + v;

    // Add the data to the dataset
    benchmarkData[cpu][name][lang].push({
      y: avg,
      x: date,
      label,
    });
  });

  return benchmarkData;
};

/**
 * Create a plugins for Chart.js
 * @returns {Object} plugins
 */
const createChartPlugins = () => {
  const horizontalHoverLinePlugin = {
    id: "horizontalHoverLine",
    beforeDraw: (chart) => {
      if (chart.tooltip?._active?.length) {
        const activePoint = chart.tooltip._active[0];
        const { ctx } = chart;
        const { y, options } = activePoint.element;
        const leftX = chart.scales.x.left;
        const rightX = chart.scales.x.right;

        // Draw vertical line
        ctx.save();
        ctx.beginPath();
        ctx.moveTo(leftX, y);
        ctx.lineTo(rightX, y);
        ctx.lineWidth = 1;
        ctx.strokeStyle = options?.borderColor
          ? options.borderColor
          : "rgba(128, 128, 128, 0.5)";
        ctx.stroke();
        ctx.restore();
      }
    },
  };

  return { horizontalHoverLinePlugin };
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

  const options = {
    responsive: true,
    maintainAspectRatio: false,
  };

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
      title: {
        display: true,
        text: "avg. time (s)",
      },
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
      title: {
        display: true,
        text: "Date",
      },
      type: "category",
      ticks: {
        font: {
          size: FONT_SIZE_SCALES,
        },
      },
      grid: {
        lineWidth: GRID_LINE_WIDTH,
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

  return { ...options, layout, scales, plugins };
};
