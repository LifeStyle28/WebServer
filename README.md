# Сборка и запуск с локалки

* Создаем все необходимые компоненты (директория build): ```conan install .. --build=missing -s compiler.libcxx=libstdc++11 -s build_type=Release && cmake .. -DCMAKE_BUILD_TYPE=Release```
* Запускаем сервер из корня (не из директории build): ```./build/bin/web_server -c templates/config.json -r result -s script.py -w web```
* Проверяем, что сервер отвечает на запрос: ```curl -v -X GET -H "Content-type: application/json" -H "Accept: application/json" -d '{"contractType":"RUSTONN_PHYS_PERSON", "currencyType":"ROUBLES", "currencyKind":"CASH", "contractDuration":2}' http://localhost:8080/api/v1/prog/tag_values```

# Запуск из-под Docker

* Создать образ с помощью команды: ```docker build -t web_server .```
* Запустить сервер с помощью команды: ```docker run --rm -p80:8080 web_server```
    - Здесь перенаправляем с порта 8080 на 80
    - ```--rm``` — указывает, что контейнер следует удалить после завершения работы в нём. Образ при этом не будет затронут.
* На релизе требуется запускать сервер с помощью команды: ```docker run --restart on-failure -p80:8080 web_server```

# Запуск тестов

* ```pip install -U pytest && pip install -U docker```
* Запускаются тесты скриптом ```run.sh``` в директории ```tests```
