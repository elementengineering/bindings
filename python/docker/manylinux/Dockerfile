# Get python builds from manylinux project
FROM quay.io/pypa/manylinux1_x86_64:latest

# Run common docker
ADD docker-common.sh /docker-common.sh
RUN bash docker-common.sh && rm -f docker-common.sh

# manylinux is Centos/5 which does not work out of the box

# Install development libraries
RUN yum -y install \
        which \
        glibc-devel \
        libstdc++-devel \
        mlocate \
        unzip \
        zip \
        gcc \
        gcc-c++ \
        zlib-devel
        
# Get glibc-2.10
RUN curl https://codeload.github.com/bminor/glibc/tar.gz/glibc-2.10 -o glibc.tar.gz && \
    tar -xvf glibc.tar.gz && \  
    (rm -rf /opt/rh/devtoolset-2/ && yum remove -y devtoolset-2-*) || true

# Compile glibc and extract object files
RUN cd glibc-glibc-2.10/ && mkdir -p build && cd build && \
    ../configure --prefix=/opt/glibc2.10 --enable-shared && \
    make -j && \
    make -j install && \
    cp libc_pic.a /opt/glibc2.10/lib/

# Compile and install libuv
RUN curl https://codeload.github.com/libuv/libuv/tar.gz/v1.10.2 -o libuv.tar.gz && \
    yum -y install libtool && \
    tar -xvf libuv.tar.gz && cd libuv-1.10.2 && \
    sh autogen.sh && \
    ./configure && \
    make -j && \
    make -j install
    
# Compile and install newer Perl in order to compile openssl
RUN curl -O http://www.cpan.org/src/perl-5.12.3.tar.gz && \
    tar -xvf perl-5.12.3.tar.gz && \
    cd perl-5.12.3 && \
    ./Configure -des -Dprefix=/opt/perl5.12 && \
    (make || true) && \
    (make install || true) 
    
# Compile and install newest openssl
RUN curl https://codeload.github.com/openssl/openssl/tar.gz/OpenSSL_1_1_1-pre2 -o openssl.tar.gz && \
    tar -xvf openssl.tar.gz && \
    cd openssl-OpenSSL_1_1_1-pre2/ && \
    export PATH=/opt/perl5.12/bin/:$PATH && \
    ./config --prefix=/opt/openssl --openssldir=/opt/openssl/usr/local/ssl && \
    make -j && \
    make -j install
    
# Upgrade gcc and g++ but patch ld
RUN curl https://people.centos.org/tru/devtools-2/devtools-2.repo -o /etc/yum.repos.d/devtools-2.repo && \
    yum install -y devtoolset-2-gcc devtoolset-2-binutils devtoolset-2-gcc-c++ && \
    mv /opt/rh/devtoolset-2/root/usr/libexec/gcc/x86_64-CentOS-linux/4.8.2/ld /opt/rh/devtoolset-2/root/usr/libexec/gcc/x86_64-CentOS-linux/4.8.2/ld.old
    
ADD ld-patch.sh /opt/rh/devtoolset-2/root/usr/libexec/gcc/x86_64-CentOS-linux/4.8.2/ld

RUN echo 'export MANYLINUX=yes' >> /etc/bashrc && \
    echo 'export CC=/opt/rh/devtoolset-2/root/usr/bin/gcc' >> /etc/bashrc && \
    echo 'export CXX=/opt/rh/devtoolset-2/root/usr/bin/g++' >> /etc/bashrc

EXPOSE 22
ENTRYPOINT ["/usr/sbin/sshd", "-D"]
