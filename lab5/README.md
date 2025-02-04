# Temperature Monitor

Программа для считывания температуры с устройства через последовательный порт и отображения данных через веб-интерфейс.

## Возможности

- Считывание температуры с устройства через последовательный порт
- Сохранение данных в SQLite базу данных
- HTTP API для получения данных
- Веб-интерфейс с графиками:
  - Текущая температура
  - График за последние 24 часа
  - График за последние 30 дней

## Структура проекта

### Backend (C++)

- `src/temp_sensor.cpp` - симулятор температурного датчика
- `src/temp_monitor.cpp` - программа мониторинга температуры
- `src/serial_port.h` - интерфейс для работы с последовательным портом
- `src/serial_port_win.cpp` - реализация для Windows
- `src/serial_port_unix.cpp` - реализация для Unix-систем
- `src/http_server.cpp` - HTTP сервер
- `src/db_manager.cpp` - работа с базой данных

### Frontend (React + TypeScript)

- `frontend/` - веб-интерфейс на React

## Тестирование без реального устройства

### macOS/Linux

1. Установите socat:

   ```bash
   brew install socat
   ```

   или для Linux:

   ```bash
   sudo apt-get install socat
   ```

2. Создайте виртуальные последовательные порты:

   ```bash
   socat -d -d pty,raw,echo=0 pty,raw,echo=0
   ```

   Запомните пути к созданным портам (например, `/dev/ttys001` и `/dev/ttys002`)

3. Соберите проект:

   ```bash
   cd lab5
   ./build.sh
   ```

4. Запустите симулятор в одном терминале:

   ```bash
   ./build/temp_sensor /dev/ttys001  # используйте первый порт из socat
   ```

5. Запустите монитор в другом терминале:
   ```bash
   ./build/temperature_monitor /dev/ttys002  # используйте второй порт из socat
   ```

### Windows

1. Установите com0com (Null-modem emulator):

   - Скачайте com0com с [официального сайта](https://sourceforge.net/projects/com0com/)
   - Установите программу
   - Запустите Setup Command Prompt (com0com)
   - Создайте пару виртуальных портов:

   ```
   install 0 PortName=COM3
   install 1 PortName=COM4
   ```

2. Соберите проект:

   ```cmd
   cd lab5
   build.cmd
   ```

3. Запустите симулятор в одном окне командной строки:

   ```cmd
   build\temp_sensor.exe COM3
   ```

4. Запустите монитор в другом окне:
   ```cmd
   build\temperature_monitor.exe COM4
   ```

## Веб-интерфейс

После запуска монитора, веб-интерфейс будет доступен по адресу: http://localhost:8080

- Текущая температура обновляется каждую секунду
- Графики обновляются каждую минуту
- Симулятор генерирует случайные значения температуры с нормальным распределением (среднее 20°C, стандартное отклонение 10°C)

## API Endpoints

- `GET /api/temperature/current` - получить текущую температуру
- `GET /api/temperature/history` - получить историю температур
  - Параметры:
    - `type`: тип данных ("raw", "hourly", "daily")
    - `start`: начальная временная метка (Unix timestamp)
    - `end`: конечная временная метка (Unix timestamp)

## Примечания

- Для корректного завершения программ используйте Ctrl+C
- База данных создается автоматически в файле `temperature.db`
- Веб-интерфейс автоматически собирается и копируется в директорию `public` при сборке проекта
