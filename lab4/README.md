# Temperature Monitor

Программа для считывания температуры с устройства через последовательный порт и ведения статистики.

## Структура проекта

- `src/temp_sensor.cpp` - симулятор температурного датчика
- `src/temp_monitor.cpp` - программа мониторинга температуры
- `src/serial_port.h` - интерфейс для работы с последовательным портом
- `src/serial_port_win.cpp` - реализация для Windows
- `src/serial_port_unix.cpp` - реализация для Unix-систем

## Тестирование без реального устройства

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
   build.cmd
   ```

3. Запустите симулятор в одном окне командной строки:

   ```cmd
   cd build\Release
   temp_sensor.exe COM3
   ```

4. Запустите монитор в другом окне:
   ```cmd
   cd build\Release
   temp_monitor.exe COM4
   ```

### Linux/macOS

1. Установите socat:

   - Ubuntu/Debian:

   ```bash
   sudo apt-get install socat
   ```

   - macOS:

   ```bash
   brew install socat
   ```

2. Создайте виртуальную пару портов:

   ```bash
   socat -d -d pty,raw,echo=0 pty,raw,echo=0
   ```

   Socat выведет пути к созданным портам, например:

   ```
   2024/03/15 12:00:00 socat[1234] N PTY is /dev/pts/1
   2024/03/15 12:00:00 socat[1234] N PTY is /dev/pts/2
   ```

3. Соберите проект:

   ```bash
   ./build.sh
   ```

4. В новом терминале запустите симулятор:

   ```bash
   cd build
   ./temp_sensor /dev/pts/1  # Используйте первый порт из вывода socat
   ```

5. В другом терминале запустите монитор:
   ```bash
   cd build
   ./temp_monitor /dev/pts/2  # Используйте второй порт из вывода socat
   ```

## Логи

Программа создает три лог-файла в директории `logs/`:

1. `raw_temp.log` - все измерения за последние 24 часа
2. `hourly_temp.log` - средние значения за каждый час последнего месяца
3. `daily_temp.log` - средние значения за каждый день текущего года

## Зависимости

### Windows

- Visual Studio 2019 или новее с поддержкой C++17
- CMake 3.10 или новее

### Linux/macOS

- GCC/Clang с поддержкой C++17
- CMake 3.10 или новее
- socat (для тестирования)

## Примечания

- Симулятор генерирует случайные значения температуры с нормальным распределением (среднее 20°C, стандартное отклонение 5°C)
- Частота обновления данных - 1 раз в секунду
- Для корректного завершения программ используйте Ctrl+C
