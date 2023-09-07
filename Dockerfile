FROM debian:bullseye-slim
RUN apt-get update && apt-get install sudo git gcc make inetutils-ping -y
RUN useradd -ms /bin/bash user && echo "user:password" | chpasswd && adduser user sudo
RUN mkdir /home/user/ft_ping
COPY . /home/user/ft_ping
RUN chown -R user:user /home/user/ft_ping
USER user
WORKDIR /home/user/ft_ping
ENTRYPOINT /bin/bash