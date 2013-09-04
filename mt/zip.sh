#!/bin/bash
target="2-WESEE.zip"
db="train.bin"
rm $target
zip -r $target src output $db Makefile README -x "*/.*" "*.o" "output/*.jpg" "output/*.JPG"
