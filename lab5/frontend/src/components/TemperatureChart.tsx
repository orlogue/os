import React from "react";
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
} from "chart.js";
import { Line } from "react-chartjs-2";
import type { TemperatureReading } from "../types";

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend
);

type Props = {
  data: TemperatureReading[];
  title: string;
};

const TemperatureChart: React.FC<Props> = ({ data, title }) => {
  console.log(`Rendering ${title} with data:`, data);

  if (!data || data.length === 0) {
    console.log(`No data available for ${title}`);
    return <div>No data available</div>;
  }

  const chartData = {
    labels: data.map((reading) => reading.formatted_time),
    datasets: [
      {
        label: "Temperature (°C)",
        data: data.map((reading) => reading.temperature),
        borderColor: "rgb(75, 192, 192)",
        backgroundColor: "rgba(75, 192, 192, 0.5)",
        tension: 0.1,
        pointRadius: 4,
        pointHoverRadius: 6,
      },
    ],
  };

  const options = {
    responsive: true,
    maintainAspectRatio: false,
    plugins: {
      legend: {
        position: "top" as const,
      },
      title: {
        display: true,
        text: title,
      },
      tooltip: {
        callbacks: {
          label: function (context: any) {
            return `Temperature: ${context.raw.toFixed(2)}°C`;
          },
        },
      },
    },
    scales: {
      y: {
        type: "linear" as const,
        beginAtZero: false,
        title: {
          display: true,
          text: "Temperature (°C)",
        },
        ticks: {
          callback: function (tickValue: number | string) {
            const value =
              typeof tickValue === "string" ? parseFloat(tickValue) : tickValue;
            return `${value.toFixed(2)}°C`;
          },
        },
      },
      x: {
        type: "category" as const,
        title: {
          display: true,
          text: title.includes("Hourly") ? "Time" : "Date",
        },
        offset: true,
        grid: {
          offset: true,
        },
      },
    },
  };

  return (
    <div style={{ width: "100%", height: "400px" }}>
      <Line data={chartData} options={options} />
    </div>
  );
};

export default TemperatureChart;
