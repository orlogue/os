cd "$(dirname "$0")/.."

git pull

mkdir -p build

cd build

cmake ..

cmake --build .

echo "Сборка завершена. Исполняемый файл находится в директории build/" 