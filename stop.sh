#!/bin/bash

processes=("loginServer/loginServer" "messageServer/messageServer" "./proxyServer/proxyServer")

stop_processes() {
    for proc in "${processes[@]}"; do
        echo "Stopping process: $proc"
        pids=$(pgrep -f "$proc")
        
        if [ -z "$pids" ]; then
            echo "No process found for: $proc"
            continue
        fi

        kill -TERM $pids 9>/dev/null

        sleep 1
        for pid in $pids; do
            if ps -p "$pid" >/dev/null 2>&1; then
                echo "Failed to stop PID $pid"
            else
                echo "Successfully stopped PID $pid"
            fi
        done
    done
}

stop_processes
