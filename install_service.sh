#!/bin/bash

set -e

if [ "$(id -u)" -ne "0" ]; then
	echo "This install script requires root."
	exit 1
fi

if [ ! -f fan-monitor.service ]; then
    echo "Service file is missing!"
    exit 1
fi

if [ -f /usr/lib/systemd/system/fan-monitor.service ]; then
    echo "Service is already installed!"
    exit 1
fi

chmod +x ./fan-monitor.service
cp -avf ./fan-monitor.service /usr/lib/systemd/system/fan-monitor.service
systemctl enable fan-monitor
systemctl start fan-monitor
