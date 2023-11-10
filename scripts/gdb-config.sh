#!/bin/bash

if [ $EUID -eq 0 ]; then 
	return 1
fi
	
wget https://raw.githubusercontent.com/hugsy/gef/main/scripts/gef.sh
chmod gef.sh
. gef.sh

