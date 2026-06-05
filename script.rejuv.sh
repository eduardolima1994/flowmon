#!/bin/bash

echo "Rejuvenation Script Started"
echo "Strategy: time"-based
echo "Action: restart_service"

echo "Starting continuous timer loop..."
while true; do
  echo "Waiting 5400 seconds before action..."
  for ((i=0; i<5400; i++)); do
    sleep 1
    if ! pgrep -f "Device1_HVGNTC_0b3592d8-7666-40b3-b5e9-b233d4cc9104_20260605145936.sh" > /dev/null; then
      echo "Main monitoring script stopped. Canceling rejuvenation."
      exit 0
    fi
  done

  echo "Restarting: aging_process"
  aging_process --quit 2>/dev/null || true
  sleep 4
  if pgrep -x aging_process >/dev/null; then
    echo "aging_process did not close, finishing remaining processes..."
    pkill -x aging_process
    sleep 2
  fi
  while pgrep -x aging_process >/dev/null; do
    sleep 1
  done
  echo "All aging_process processes completed"
  LOGGED_USER=$(who | head -n 1 | awk '{print $1}')
  echo "User detected: $LOGGED_USER"
  LOGGED_UID=$(id -u $LOGGED_USER)
  echo "UID: $LOGGED_UID"
  if [ -S "/run/user/$LOGGED_UID/wayland-0" ]; then
    echo "Wayland detected"
    export WAYLAND_DISPLAY=wayland-0
    export XDG_RUNTIME_DIR=/run/user/$LOGGED_UID
  else
    echo "X11 detected or assumed"
    export DISPLAY=:0
    export XAUTHORITY=/home/$LOGGED_USER/.Xauthority
  fi
  export DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$LOGGED_UID/bus
  echo "Starting aging_process..."
  echo "my_password" | sudo -S -u $LOGGED_USER -i bash -c "export DISPLAY=:0 WAYLAND_DISPLAY=wayland-0 XDG_RUNTIME_DIR=/run/user/$LOGGED_UID DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$LOGGED_UID/bus; nohup aging_process >/dev/null 2>&1 &"
  sleep 5
  if pgrep -f "aging_process" > /dev/null 2>&1; then
    NEW_PID=$(pgrep -f "aging_process" | head -n 1)
    echo "aging_process running (PID: $NEW_PID)"
  else
    echo "Failed to start. Debug:"
    echo "User: $LOGGED_USER (UID: $LOGGED_UID)"
    echo "Active graphic sessions:"
    loginctl list-sessions 2>/dev/null || echo "    loginctl not available"
    echo "Attempting to start manually (check logs):"
    sudo -u $LOGGED_USER aging_process 2>&1 | head -n 5
  fi
  echo "Timer cycle completed - restarting timer..."
done