#/bin/bash

if [ "$HOME" == "/root" ]; then
	echo "No development in /root directory";
	exit 0;
fi

DEVROOT="/home/$USER/linux"

echo "obj-\$(CONFIG_COIN)	+= coin.o" >> ${DEVROOT}/drivers/char/Makefile
cat << EOF >> ${DEVROOT}/drivers/char/Kconfig
config COIN
	tristate "Coin Flipping character device"
	help
		Y for builtin, M for module. If unsure, say N.
EOF


