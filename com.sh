#!/bin/sh

[ -z "$1" ] && exit

while IFS= read -r line; do
    mkdir "$1/$line" && dd if=/dev/zero of="$1/$line/index.html" bs=1 count=$(expr $(date +%N) % 10240)
done < common.txt
