FROM debian:bullseye-slim
RUN apt-get update && apt-get install sudo git gcc make inetutils-ping -y
RUN useradd -ms /bin/bash user && echo "user:password" | chpasswd && adduser user sudo
USER user
RUN mkdir /home/user/ft_ping
WORKDIR /home/user/ft_ping
ENTRYPOINT /bin/bash