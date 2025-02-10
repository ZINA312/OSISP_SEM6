#! /bin/bash

input_file="input.txt"
output_file="output.txt"

# Проверка существования входного файла
if [[ ! -f "$input_file" ]]; then
    echo "Ошибка: файл $input_file не найден."
    exit 1
fi

# Проверка доступности файла для чтения
if [[ ! -r "$input_file" ]]; then
    echo "Ошибка: файл $input_file недоступен для чтения."
    exit 1
fi

# Обработка текста
sed -E 's/(^|\. |\! |\? )([a-z])/\1\u\2/g' "$input_file" > "$output_file"

# Проверка успешности выполнения
if [[ $? -eq 0 ]]; then
    echo "Текст успешно обработан. Результат сохранен в $output_file."
else
    echo "Ошибка при обработке текста."
    exit 1
fi