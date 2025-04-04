#!/bin/bash

./loginServer/loginServer -daemon &
./messageServer/messageServer -daemon &
./proxyServer/server -daemon &

wait