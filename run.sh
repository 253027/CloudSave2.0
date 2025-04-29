#!/bin/bash

./loginServer/loginServer -daemon &
./messageServer/messageServer -daemon &
./proxyServer/proxyServer -daemon &
./redis/bin/redis-server ./redis.conf &

wait