#!/bin/bash
KERNEL_PATH=$1
DISK_PATH=$2

# Start QEMU in the background
echo "Starting QEMU (DISPLAY=$DISPLAY)"
qemu-system-i386 -S -gdb tcp::1234 -cdrom /workspaces/2026-ikt218-osdev/build/14_marthe-sigrid/kernel.iso -m 64 -serial pty -machine pcspk-audiodev=snd -audiodev pa,id=snd > /tmp/qemu.log 2>&1 &
QEMU_PID=$!

# Give QEMU a moment to either start or crash
sleep 1
if ! kill -0 $QEMU_PID 2>/dev/null; then
    echo "QEMU crashed on startup. Log:"
    cat /tmp/qemu.log
    exit 1
fi
echo "QEMU process alive (pid=$QEMU_PID), waiting for port 1234..."

# Wait until QEMU is actually listening on port 1234
WAIT=0
while ! python3 -c "import socket,sys; s=socket.socket(); s.settimeout(0.2); sys.exit(0 if s.connect_ex(('localhost',1234))==0 else 1)" 2>/dev/null; do
    sleep 0.2
    WAIT=$((WAIT+1))
    if [ $WAIT -gt 30 ]; then
        echo "Timeout waiting for port 1234. QEMU log:"
        cat /tmp/qemu.log
        exit 1
    fi
done
echo "QEMU ready"

# Function to check if gdb is running
is_gdb_running() {
    pgrep -f "gdb-multiarch" > /dev/null
}

# Function to handle termination signals
cleanup() {
    echo "Stopping QEMU..."
    kill $QEMU_PID
    exit 0
}

# Trap SIGINT and SIGTERM signals
trap cleanup SIGINT SIGTERM

# Wait for gdb to start
echo "Waiting for gdb to start..."
while ! is_gdb_running; do
    sleep 1
done
echo "gdb started."

# Wait for gdb to stop
echo "Monitoring gdb connection..."
while is_gdb_running; do
    sleep 1
done

# Cleanup after gdb stops
cleanup
