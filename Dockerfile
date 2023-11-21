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
COPY CMakeLists.txt /app/
COPY main.cpp /app/
COPY ./server_certificate.h /app

RUN cd /app/build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . -j12

# Второй контейнер в том же докерфайле
FROM ubuntu:22.04 as run

ENV DEBIAN_FRONTEND noninteractive.
RUN apt-get update
RUN apt-get install -y language-pack-ru p7zip-full
ENV LANGUAGE ru_RU.UTF-8
ENV LANG ru_RU.UTF-8
ENV LC_ALL ru_RU.UTF-8
RUN locale-gen ru_RU.UTF-8 && dpkg-reconfigure locales

# Установим python v3 и pip
RUN apt update && apt install -y python3 python3-pip

# Установим необходимые для скрипта модули
RUN pip install docxcompose python-docx num2words python-dateutil

# Setup OpenSSL
RUN apt update && apt install -y wget
ARG OPENSSL_VERSION=1.1.1d
RUN wget https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz
RUN tar -zxvf openssl-${OPENSSL_VERSION}.tar.gz
# Fix error `openssl: error while loading shared libraries: libssl.so.1.1: cannot open shared object file: No such file or directory`. Reference: https://github.com/openssl/openssl/issues/3993
RUN cd openssl-${OPENSSL_VERSION} && \
    ./config \
    --debug \
    --prefix=/usr/local \
    --libdir=/lib \
    --openssldir=/usr/local/ssl && \
    make && make install
# Add /usr/local/openssl/lib to /etc/ld.so.conf and then run the command `ldconfig`
RUN echo '/usr/local/ssl/lib' >> /etc/ld.so.conf
RUN ldconfig
RUN echo 'export LD_LIBRARY_PATH=/usr/local/ssl/lib' >> ~/.bash_profile && . ~/.bash_profile
RUN openssl version

RUN mkdir /app && chmod 777 -R /app
COPY ./templates /app/templates
RUN chmod 777 /app/templates/config.json
COPY ./certs /app/certs
RUN chmod 777 -R /app/certs

# Создадим пользователя admin
RUN groupadd -r admin && useradd -mrg admin admin
USER admin

# Создадим рабочую папку app и выставим права доступа
RUN mkdir -p /app/templates && mkdir /app/web && mkdir /app/web/result && mkdir /app/script

# Скопируем приложение из сборочного контейнера в директорию /app
COPY --from=build /app/build/bin/web_server /app/
COPY --from=build /app/build/lib/* /app/
COPY ./script /app/script
COPY ./web /app/web
ENV LD_LIBRARY_PATH=/app/

# Запускаем веб-сервер
ENTRYPOINT ["/app/web_server", "-r", "/app/web/result/", "-c", "/app/templates/config.json", "-s",\
    "/app/script/script.py", "-w", "/app/web/", "-v", "/app/certs"]
