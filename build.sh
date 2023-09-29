#!/bin/bash

if [ $# -ne 1 ]; then
	echo "Usage: ./$0 <kernel tarball link>"
	exit -1
fi

URL=$1
KERN_VER="kernel-dir"
PROC_COUNT=$(lscpu | awk '/^CPU\(s\)/ {print $2}')

echo 'set -o vi' >> ~/.bashrc
echo 'hi comment ctermfg=red' >> ~/.vimrc

if which apt; then 
	sudo apt -y update && sudo apt -y upgrade
	sudo apt -y install git fakeroot build-essential \
		ncurses-dev xz-utils libssl-dev bc flex \
		libelf-dev bison qemu-system-x86 gdb dwarves
fi;

wget -O ${KERN_VER}.tar.xz $1
mkdir ${KERN_VER}
tar -xf ${KERN_VER}.tar.xz -C ${KERN_VER}/ --strip-components=1

cp /boot/config-$(uname -r) ${KERN_VER}/.config

sudo make -C ${KERN_VER} kvm_guest.config
sudo sed -i '/CONFIG_RANDOMIZE_BASE/s/^/#/' ${KERN_VER}/.config
sudo sed -i '/CONFIG_TRUSTED_KEYS/s/^/#/' ${KERN_VER}/.config
sudo sed -i '/CONFIG_SYSTEM_TRUSTED_KEYRING/s/^/#/' ${KERN_VER}/.config
sudo sed -i '/CONFIG_SYSTEM_TRUSTED_KEYS/s/^/#/' ${KERN_VER}/.config
sudo sed -i '/CONFIG_SYSTEM_REVOCATION_LIST/s/^/#/' ${KERN_VER}/.config
sudo sed -i '/CONFIG_SYSTEM_REVOCATION_KEYS/s/^/#/' ${KERN_VER}/.config
sudo echo 'CONFIG_KASAN=y' >> ${KERN_VER}/.config
sudo echo 'CONFIG_KASAN_INLINE=y' >> ${KERN_VER}/.config

make -C ${KERN_VER} savedefconfig
make -C ${KERN_VER} -j${PROC_COUNT}

