# Не просто создаём образ, но даём ему имя build
FROM gcc:11.3 as build

RUN apt update && \
    apt install -y \
      python3-pip \
      cmake \
    && \
    pip3 install conan==1.59.0

# Запуск conan
COPY conanfile.txt /app/
RUN mkdir /app/build && cd /app/build && \
    conan install .. --build=missing -s compiler.libcxx=libstdc++11 -s build_type=Release

# Копируем все необходимые компоненты
COPY ./application /app/application
COPY ./http_server /app/http_server
COPY ./logger /app/logger
COPY ./templates /app/templates
COPY ./script /app/script
COPY ./result /app/result
COPY CMakeLists.txt /app/
COPY main.cpp /app/

RUN cd /app/build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . -j12

# Второй контейнер в том же докерфайле
FROM ubuntu:22.04 as run

# Создадим рабочую папку app и выставим права доступа
RUN mkdir -p /app
RUN chmod 777 -R /app

# Установим python v3 и pip
RUN apt update && apt install -y python3 python3-pip

# Создадим пользователя admin
RUN groupadd -r admin && useradd -mrg admin admin
USER admin

# Установим необходимые для скрипта модули
RUN pip install docxcompose python-docx num-to-rus python-dateutil

# Скопируем приложение из сборочного контейнера в директорию /app
COPY --from=build /app/build/bin/web_server /app/
COPY --from=build /app/build/lib/* /app/
ENV LD_LIBRARY_PATH=/app/

# Запускаем веб-сервер
ENTRYPOINT ["/app/web_server", "--result-path", "/app/result/", "--config-file", "/app/templates/config.json", "-s", "/app/script/script.py"]
