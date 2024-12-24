@echo off

cd /d "%~dp0\.."

git pull

if not exist build mkdir build

cd build

cmake ..

cmake --build .

echo Сборка завершена. Исполняемый файл находится в директории build/
pause 