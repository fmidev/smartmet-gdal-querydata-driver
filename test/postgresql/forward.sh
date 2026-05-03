#! /bin/sh

set -x

while true; do
    /usr/bin/timeout 60 socat TCP-LISTEN:15432,fork,reuseaddr TCP-CONNECT:smartmet-test:5444
    sleep 15
done
