#!/bin/bash

# Get the real user ID (works even when script is run with sudo)
if [ -n "$SUDO_USER" ]; then
    REAL_USER=$SUDO_USER
    REAL_UID=$(id -u $SUDO_USER)
    REAL_GID=$(id -g $SUDO_USER)
else
    REAL_USER=$(whoami)
    REAL_UID=$(id -u)
    REAL_GID=$(id -g)
fi

echo "Fixing ownership for user: $REAL_USER ($REAL_UID:$REAL_GID)"

sudo chown -R $REAL_UID:$REAL_GID build
sudo chmod -R u+rw build

echo "✓ Ownership fixed"
