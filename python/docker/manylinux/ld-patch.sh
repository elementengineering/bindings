#!/bin/bash
set -e
set -o pipefail
args=$(echo $@ | sed -e 's/--build-id//g')
#for f in crtbeginS.o crtendS.o crtbeginT.o; do
#	HACK_PATH="/opt/rh/devtoolset-2/root/usr/lib/gcc/x86_64-CentOS-linux/4.8.2/$f"
#	ORIG_PATH="/usr/lib/gcc/x86_64-redhat-linux/4.1.1/$f"
#	args=$(echo $args | sed -e "s%${HACK_PATH}%${ORIG_PATH}%g")
#done
#args=$(echo $args | sed -e "s%-L/opt/rh/devtoolset-2/root/usr/lib/gcc/x86_64-CentOS-linux/4.8.2\S*%%g")

args="/usr/lib64/crti.o $args"
args="/usr/lib/gcc/x86_64-redhat-linux/4.1.1/crtbeginS.o $args"
args="$args /usr/lib/gcc/x86_64-redhat-linux/4.1.1/crtendS.o"
args="$args /usr/lib64/crtn.o"
args="$args --start-group -lc -lm -lpthread -lgcc_s -lc -lstdc++ --end-group"

echo -e "\n\nRun linker:\t$args\n\n" 2>&1 | tee /vagrant/debug.ld
ld $args --verbose 2>&1 | tee -a /vagrant/debug.ld
