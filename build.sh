#!/bin/bash

if [ $# -ne 1 ]; then
	# TODO - verify this is URL
	echo "Usage: ./$0 <kernel tarball link>"
	exit -1
fi

KERN_DIR="kernel-dir"
PROC_COUNT=$(($(nproc) * 2))

echo 'set -o vi' >> ~/.bashrc
echo 'hi comment ctermfg=red' >> ~/.vimrc

if which apt; then 
	sudo apt -y update && sudo apt -y upgrade
	sudo apt -y install git fakeroot build-essential \
		ncurses-dev xz-utils libssl-dev bc flex \
		libelf-dev bison qemu-system-x86 gdb dwarves \
  		debootstrap
fi;

if which yum; then
	sudo yum -y update
	sudo yum -y install autoconf automake binutils bison \
   	flex gcc gdb glibc-devel libtool make pkgconf \
   	pkgconf-m4 pkgconf-pkg-config redhat-rpm-config qemu-kvm
	sudo yum -y install epel-release
	#recall this working on 9 and failing on 8
	if sudo yum config-manager --set-enabled powertools; then
		sudo yum -y install dwarves 
	else
		echo "Manually install PowerTools repo"
		RELEASE=$(cat /etc/*release)
		echo "$RELEASE"
		exit 2
	fi
fi

wget -O ${KERN_DIR}.tar.xz $1
mkdir ${KERN_DIR}
tar -xf ${KERN_DIR}.tar.xz -C ${KERN_DIR}/ --strip-components=1

cp /boot/config-$(uname -r) ${KERN_DIR}/.config

make -C ${KERN_DIR} kvm_guest.config
sed -i '/CONFIG_RANDOMIZE_BASE/s/^/#/' ${KERN_DIR}/.config
sed -i '/CONFIG_TRUSTED_KEYS/s/^/#/' ${KERN_DIR}/.config
sed -i '/CONFIG_SYSTEM_TRUSTED_KEYRING/s/^/#/' ${KERN_DIR}/.config
sed -i '/CONFIG_SYSTEM_TRUSTED_KEYS/s/^/#/' ${KERN_DIR}/.config
sed -i '/CONFIG_SYSTEM_REVOCATION_LIST/s/^/#/' ${KERN_DIR}/.config
sed -i '/CONFIG_SYSTEM_REVOCATION_KEYS/s/^/#/' ${KERN_DIR}/.config
sed -i '/CONFIG_FRAME_WARN=1024/c CONFIG_FRAME_WARN=2048' ${KERN_DIR}/.config
echo 'CONFIG_KASAN=y' >> ${KERN_DIR}/.config
echo 'CONFIG_KASAN_INLINE=y' >> ${KERN_DIR}/.config

make -C ${KERN_DIR} savedefconfig
make -C ${KERN_DIR} -j${PROC_COUNT}

