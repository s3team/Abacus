FROM ubuntu:18.04


RUN apt-get update && apt-get upgrade -y

RUN apt-get install -y -q wget \
software-properties-common

RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null

RUN apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'

RUN apt-get update && apt-get upgrade -y

RUN apt-get install -y -q \
git \
autoconf \
automake \
debianutils \
m4 \
build-essential \
automake \
autoconf \
libtool \
wget \
python3 \
libssl-dev \
libcurl4-openssl-dev \
curl \
gcc \
cmake \
pkg-config



