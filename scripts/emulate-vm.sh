#!/bin/bash

if which qemu-system-x86_64; then
  qemu-system-x86_64 -kernel ~/supreme-umbrella/kernel-dir/arch/x86_64/boot/bzImage \
  -append "console=ttyS0 root=/dev/sda earlyprintk=serial nokaslr" -hda bullseye.img \
  -enable-kvm -nographic -m 1G -s -S -smp 2
else   
  echo "qemu-system-x86_64 not installed"
  return -1
fi
  
