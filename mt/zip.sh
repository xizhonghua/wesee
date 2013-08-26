#!/bin/bash
target="2-WESEE.zip"
rm $target
zip -r $target src Makefile README -x "*/.*" "*.o"
