#!/bin/sh

user='verik'
ip_list=$1
TS=$(date +%s)

[ -f linpeas.sh ] || wget https://github.com/carlospolop/PEASS-ng/releases/latest/download/linpeas.sh 2>/dev/null

mkdir -p output/peas

for ip in $(cat ${ip_list}) ; do
    scp -o StrictHostKeyChecking=no linpeas.sh ${user}@${ip}: 2>/dev/null
    ssh -o StrictHostKeyChecking=no ${user}@${ip} 'chmod +x linpeas.sh ; sh -c ~/linpeas.sh ; rm linpeas.sh' > output/peas/${ip}-${TS}-peas.txt 2>&1 &
done
