#!/bin/bash

./loginServer/loginServer -daemon &
./messageServer/messageServer -daemon &
./proxyServer/proxyServer -daemon &

wait