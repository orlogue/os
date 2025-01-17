export type TemperatureReading = {
  timestamp: number;
  temperature: number;
  formatted_time: string;
};

export type TemperatureResponse = {
  data: TemperatureReading[];
};

export type CurrentTemperature = {
  temperature: number;
  timestamp: number;
};
