FROM debian:bullseye-slim
RUN apt-get update && apt-get install git vim man gcc make inetutils-ping -y
RUN mkdir /root/ft_ping
WORKDIR /root/ft_ping
ENTRYPOINT /bin/bash