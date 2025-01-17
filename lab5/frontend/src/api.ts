import axios from "axios";
import type { TemperatureResponse, CurrentTemperature } from "./types";

const API_BASE_URL = "http://localhost:8080/api";

export const fetchCurrentTemperature =
  async (): Promise<CurrentTemperature> => {
    const response = await axios.get<CurrentTemperature>(
      `${API_BASE_URL}/temperature/current`
    );
    return response.data;
  };

export const fetchTemperatureHistory = async (
  type: string,
  startTime: number,
  endTime: number
): Promise<TemperatureResponse> => {
  const response = await axios.get<TemperatureResponse>(
    `${API_BASE_URL}/temperature/history`,
    {
      params: {
        type,
        start: startTime,
        end: endTime,
      },
    }
  );
  return response.data;
};
