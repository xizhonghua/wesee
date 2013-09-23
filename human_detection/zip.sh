#!/bin/bash
target="1-WESEE.zip"
db="person.xml"
rm $target
zip -q -9 -r $target main.cpp src util third_party boost_1_50_0 include $db Makefile README -x "*/.*" "*.o" "output/*.jpg" "output/*.JPG"
