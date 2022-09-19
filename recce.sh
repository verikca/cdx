#!/bin/sh

user='verik'
ip_list=$1
TS=$(date +%s)

mkdir -p output/ps

for ip in $(cat ${ip_list}) ; do
    mkdir -p output/${TS}/${ip}

    ssh -o StrictHostKeyChecking=no ${user}@${ip} 'sudo grep -r . /etc/passwd /etc/shadow /etc/group /etc/gshadow /etc/sudoers /etc/sudoers.d /etc/pam.conf /etc/pam.d /etc/cron* /var/spool/cron' > output/${TS}/${ip}/auth_config.txt 2>/dev/null &

    ssh -o StrictHostKeyChecking=no ${user}@${ip} 'sudo netstat -nlptu' > output/${TS}/${ip}/ports.txt 2>/dev/null &

    ssh -o StrictHostKeyChecking=no ${user}@${ip} 'sudo systemctl -a' > output/${TS}/${ip}/systemctl.txt 2>/dev/null &

    ssh -o StrictHostKeyChecking=no ${user}@${ip} 'sudo ps -lax' > output/ps/${ip}-${TS}-ps.txt 2>/dev/null &

    ssh -o StrictHostKeyChecking=no ${user}@${ip} 'sudo find /proc -name exe -printf \"%l\"\\n' 2>/dev/null | sort -u > output/${TS}/${ip}/ps_exe.txt &

    ssh -o StrictHostKeyChecking=no ${user}@${ip} 'sudo find / -type f,d,l ! -fstype proc -printf %i,%n,%s,%T@,%C@,%d,%u,%g,%m,%M,%F,\"%h%p\",\"%l\"\\n' > output/${TS}/${ip}/find.txt 2>/dev/null &

    ssh -o StrictHostKeyChecking=no ${user}@${ip} 'sudo lsattr -R /' 2>/dev/null | grep -v ':$' | grep -v '^$' > output/${TS}/${ip}/attr.txt &

done
