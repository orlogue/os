import React, { useEffect, useState } from "react";
import TemperatureChart from "./components/TemperatureChart";
import { fetchCurrentTemperature, fetchTemperatureHistory } from "./api";
import type { TemperatureReading, CurrentTemperature } from "./types";

const formatTimestamp = (
  timestamp: number,
  type: "hourly" | "daily"
): string => {
  console.log(`Formatting timestamp: ${timestamp} for type: ${type}`);
  const date = new Date(timestamp * 1000);
  if (type === "hourly") {
    return date.toLocaleTimeString([], {
      hour: "2-digit",
      minute: "2-digit",
      hour12: false,
    });
  }
  return date.toLocaleDateString();
};

const App: React.FC = () => {
  const [currentTemp, setCurrentTemp] = useState<CurrentTemperature | null>(
    null
  );
  const [hourlyData, setHourlyData] = useState<TemperatureReading[]>([]);
  const [dailyData, setDailyData] = useState<TemperatureReading[]>([]);

  const fetchCurrent = async () => {
    try {
      const data = await fetchCurrentTemperature();
      console.log("Current temperature data:", data);
      setCurrentTemp({
        ...data,
        temperature: Number(data.temperature.toFixed(2)),
      });
    } catch (error) {
      console.error("Failed to fetch current temperature:", error);
    }
  };

  const fetchHistory = async () => {
    try {
      const now = Math.floor(Date.now() / 1000);
      const currentHour = now - (now % 3600);

      const hourlyStart = currentHour - 24 * 3600;
      const hourlyEnd = currentHour + 3600;

      const currentDay = now - (now % 86400);
      const dailyStart = currentDay - 30 * 86400;
      const dailyEnd = currentDay + 86400;

      console.log(
        "Hourly range:",
        new Date(hourlyStart * 1000).toISOString(),
        "to",
        new Date(hourlyEnd * 1000).toISOString(),
        `(${hourlyStart} to ${hourlyEnd})`
      );
      console.log(
        "Daily range:",
        new Date(dailyStart * 1000).toISOString(),
        "to",
        new Date(dailyEnd * 1000).toISOString(),
        `(${dailyStart} to ${dailyEnd})`
      );

      const hourlyResponse = await fetchTemperatureHistory(
        "hourly",
        hourlyStart,
        hourlyEnd
      );
      const dailyResponse = await fetchTemperatureHistory(
        "daily",
        dailyStart,
        dailyEnd
      );

      console.log("Raw hourly response:", hourlyResponse);
      console.log("Raw daily response:", dailyResponse);

      const formattedHourlyData = hourlyResponse.data
        .sort((a, b) => a.timestamp - b.timestamp)
        .map((reading) => ({
          ...reading,
          formatted_time: formatTimestamp(reading.timestamp, "hourly"),
          temperature: Number(reading.temperature.toFixed(2)),
        }));

      const formattedDailyData = dailyResponse.data
        .sort((a, b) => a.timestamp - b.timestamp)
        .map((reading) => ({
          ...reading,
          formatted_time: formatTimestamp(reading.timestamp, "daily"),
          temperature: Number(reading.temperature.toFixed(2)),
        }));

      console.log("Formatted hourly data:", formattedHourlyData);
      console.log("Formatted daily data:", formattedDailyData);

      setHourlyData(formattedHourlyData);
      setDailyData(formattedDailyData);
    } catch (error) {
      console.error("Failed to fetch temperature history:", error);
    }
  };

  useEffect(() => {
    fetchCurrent();
    fetchHistory();

    const currentInterval = setInterval(fetchCurrent, 1000);
    const historyInterval = setInterval(fetchHistory, 60000);

    return () => {
      clearInterval(currentInterval);
      clearInterval(historyInterval);
    };
  }, []);

  return (
    <div style={{ padding: "20px", maxWidth: "1200px", margin: "0 auto" }}>
      <h1>Temperature Monitor</h1>

      <div
        style={{
          marginBottom: "20px",
          padding: "20px",
          backgroundColor: "#f5f5f5",
          borderRadius: "8px",
        }}
      >
        <h2>Current Temperature</h2>
        {currentTemp ? (
          <div style={{ fontSize: "24px", fontWeight: "bold" }}>
            {currentTemp.temperature.toFixed(2)}°C
          </div>
        ) : (
          <div>Loading...</div>
        )}
      </div>

      <div style={{ marginBottom: "40px" }}>
        <h2>Last 24 Hours</h2>
        {hourlyData.length > 0 ? (
          <TemperatureChart data={hourlyData} title="Hourly Temperature" />
        ) : (
          <div>Loading hourly data...</div>
        )}
      </div>

      <div>
        <h2>Last 30 Days</h2>
        {dailyData.length > 0 ? (
          <TemperatureChart data={dailyData} title="Daily Temperature" />
        ) : (
          <div>Loading daily data...</div>
        )}
      </div>
    </div>
  );
};

export default App;
