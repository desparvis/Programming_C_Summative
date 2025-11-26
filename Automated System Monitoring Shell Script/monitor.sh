#!/bin/bash

# default threshold values
CPU_LIMIT=80
MEM_LIMIT=80
DISK_LIMIT=80

# log file
LOG_FILE="system_log.txt"

# check if log file exists
if [ ! -f "$LOG_FILE" ]; then
    touch "$LOG_FILE"
fi

# function to get system status
get_status() {
    # get CPU %
    CPU=$(top -bn1 | grep "Cpu(s)" | awk '{print $2 + $4}' | cut -d'.' -f1)

    # get memory %
    MEM=$(free | awk '/Mem/ {printf "%.0f", $3/$2 * 100}')

    # get disk %
    DISK=$(df / | awk 'NR==2 {print $5}' | sed 's/%//')

    echo "CPU: $CPU%"
    echo "Memory: $MEM%"
    echo "Disk: $DISK%"
}

# function to log activity
log_status() {
    echo "$(date): CPU:$CPU% MEM:$MEM% DISK:$DISK%" >> "$LOG_FILE"
}

# function to check alerts
check_alerts() {
    if [ "$CPU" -ge "$CPU_LIMIT" ]; then
        echo "ALERT: CPU is high ($CPU%)" | tee -a "$LOG_FILE"
    fi

    if [ "$MEM" -ge "$MEM_LIMIT" ]; then
        echo "ALERT: Memory usage is high ($MEM%)" | tee -a "$LOG_FILE"
    fi

    if [ "$DISK" -ge "$DISK_LIMIT" ]; then
        echo "ALERT: Disk usage is high ($DISK%)" | tee -a "$LOG_FILE"
    fi
}

# function for periodic monitoring
auto_monitor() {
    echo "Auto-monitoring every 5 seconds. Press Ctrl+C to stop."
    while true; do
        CPU=$(top -bn1 | grep "Cpu(s)" | awk '{print $2 + $4}' | cut -d'.' -f1)
        MEM=$(free | awk '/Mem/ {printf "%.0f", $3/$2 * 100}')
        DISK=$(df / | awk 'NR==2 {print $5}' | sed 's/%//')

        log_status
        check_alerts
        sleep 5
    done
}

# function to set new thresholds
set_thresholds() {
    echo "Enter new CPU limit:"
    read CPU_LIMIT
    echo "Enter new Memory limit:"
    read MEM_LIMIT
    echo "Enter new Disk limit:"
    read DISK_LIMIT

    echo "Thresholds updated."
}

# main menu loop
while true; do
    echo "====================="
    echo " SYSTEM MONITOR MENU"
    echo "====================="
    echo "1. View System Status"
    echo "2. Set Alert Thresholds"
    echo "3. View Logs"
    echo "4. Clear Logs"
    echo "5. Auto Monitor"
    echo "6. Exit"
    echo "Choose an option:"
    read choice

    case $choice in
        1)
            get_status
            ;;
        2)
            set_thresholds
            ;;
        3)
            cat "$LOG_FILE"
            ;;
        4)
            > "$LOG_FILE"
            echo "Logs cleared."
            ;;
        5)
            auto_monitor
            ;;
        6)
            echo "Exiting."
            exit 0
            ;;
        *)
            echo "Invalid option."
            ;;
    esac
done