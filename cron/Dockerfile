FROM ubuntu:22.04 as cron
RUN apt update && \
    apt install -y \
      cron python3
RUN mkdir -p /result
RUN mkdir -p /cron
COPY ./cron/crontab /etc/crontab
COPY ./cron/dir_remover.py /cron
ENTRYPOINT ["cron", "-f"]