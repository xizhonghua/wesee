#!/bin/bash
target="2-WESEE.zip"
db="train.bin"
rm $target
rm output/*.jpg output/*.JPG output/*.jpeg output/*.JPEG
zip -r $target src $db Makefile README -x "*/.*" "*.o"
