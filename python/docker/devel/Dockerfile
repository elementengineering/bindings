# Docker image for Vagrant which allows vagrant ssh to work
FROM debian:buster
ADD sources.list /etc/apt/sources.list
ADD docker-common.sh /docker-common.sh
RUN bash docker-common.sh && rm -f docker-common.sh
    
RUN apt-get -y install \
        rsync \
        g++ \
        gcc \
        make \
        git \
        cmake \
        libssl-dev \
        libuv1-dev \
        python \
        python-pip \
        python3 \
        python3-pip \
        python3-all-dev \
        python2.7-dev
        
RUN pip3 install --upgrade pip && \
    pip2 install --upgrade pip

RUN pip3 install wheel && \
    pip2 install wheel
        
EXPOSE 22
ENTRYPOINT ["/usr/sbin/sshd", "-D"]
