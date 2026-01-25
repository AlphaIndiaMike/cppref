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

sudo chown -R $REAL_UID:$REAL_GID project/build
sudo chmod -R u+rw project/build

sudo chown -R $REAL_UID:$REAL_GID project/docs
sudo chmod -R u+rw project/docs

echo "✓ Ownership fixed"
