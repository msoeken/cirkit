FROM ubuntu:18.04

ENV PYENV_ROOT /root/.pyenv
ENV PATH /root/.pyenv/shims:/root/.pyenv/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

RUN apt-get update
RUN apt-get install -y cmake git gcc g++ ninja-build build-essential mercurial libssl-dev libbz2-dev libreadline-dev libsqlite3-dev curl zlib1g-dev libffi-dev
RUN curl -L https://raw.githubusercontent.com/yyuu/pyenv-installer/master/bin/pyenv-installer | bash

RUN pyenv install 2.7.15
RUN pyenv install 3.5.5
RUN pyenv install 3.6.5
RUN pyenv install 3.7.0

ENV TERM=xterm
ENV HOME /root
WORKDIR /root

COPY . /root/mockturtle
WORKDIR /root/mockturtle/dist

RUN ./build.sh

CMD ["bash"]
