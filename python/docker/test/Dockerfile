FROM centos:latest

RUN yum -y install which

# Run common docker
ADD docker-common.sh /docker-common.sh
RUN bash docker-common.sh && rm -f docker-common.sh

RUN yum -y install epel-release
RUN yum -y install python-pip

EXPOSE 22
ENTRYPOINT ["/usr/sbin/sshd", "-D"]
