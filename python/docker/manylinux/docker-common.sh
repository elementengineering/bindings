#!/bin/bash

set -v
set -x

# Running this script at the start of a Dockerfile will allow vagrant ssh to correctly work with that machine
# Should work for apt (debian) or yum (centos) based distributions
# We use this because vagrant ssh is a more convenient workflow than `docker attach`

function generic {

   groupadd -g 1000 vagrant && \
    useradd --create-home -s /bin/bash -u 1000 -g 1000 vagrant && \
    (echo 'vagrant:vagrant' | chpasswd) && \
    if [[ -d /etc/sudoers.d ]]; then SUDOERS=/etc/sudoers.d/vagrant; rm -f "$SUDOERS"; else SUDOERS=/etc/sudoers; fi && \
    (echo 'vagrant ALL = NOPASSWD: ALL' >> "$SUDOERS" && chmod 440 "$SUDOERS" && chown root:root "$SUDOERS") && \
    (mkdir -p /home/vagrant/.ssh && chmod 700 /home/vagrant/.ssh) && \
    (mkdir -p /vagrant && chown -R vagrant:vagrant /vagrant) && \
     (echo ssh-rsa AAAAB3NzaC1yc2EAAAABIwAAAQEA6NF8iallvQVp22WDkTkyrtvp9eWW6A8YVr+kz4TjGYe7gHzIw+niNltGEFHzD8+v1I2YJ6oXevct1YeS0o9HZyN1Q9qgCgzUFtdOKLv6IedplqoPkcmF0aYet2PkEDo3MlTBckFXPITAMzF8dJSIFo9D8HfdOV0IAdx4O7PtixWKn5y2hMNG0zQPyUecp4pzC6kivAIhyfHilFR61RGL+GPXQ2MWZWFYbAGjyiYJnAmCP3NOTd0jMZEnDkbUvxhMmBYSdETk1rRgm+R4LOzFUGaHqHDLKLX+FIPKcF96hrucXzcWyLbIbEgE98OHlnVYCzRdK8jlqm8tehUc9c9WhQ== vagrant insecure public key > /home/vagrant/.ssh/authorized_keys && chmod 600 /home/vagrant/.ssh/authorized_keys) && \
    chmod 600 /home/vagrant/.ssh/authorized_keys && \
    chown -R vagrant:vagrant /home/vagrant/.ssh && \
    mkdir -p /run/sshd && \
    sed -i -e 's/Defaults.*requiretty/#&/' /etc/sudoers && \
    sed -i -e 's/#\?\(UsePAM \).*/\1 no/' /etc/ssh/sshd_config && \
    sed -i -e 's/#\?\(UseDNS \).*/\1 no/' /etc/ssh/sshd_config && \
    sed -i -e 's/#\?\(GSSAPIAuthentication \).*/\1 no/' /etc/ssh/sshd_config && \
    sed -i -e 's/#\?\(PermitRootLogin \).*/\1 without-password/' /etc/ssh/sshd_config && \
    (if ! [[ -f /etc/ssh/ssh_host_rsa_key ]]; then \
        ssh-keygen -t rsa -f /etc/ssh/ssh_host_rsa_key; \
        ssh-keygen -t dsa -f /etc/ssh/ssh_host_dsa_key; \
    fi)
    return $?
}

if ( which apt-get ); then 
    apt-get -y update && apt-get -y install sudo && apt-get -y install openssh-server && generic
    exit $?
fi

if ( which yum ); then
    yum -y update && yum -y install sudo && yum -y install openssh-server openssh-clients && generic
    exit $?
fi
