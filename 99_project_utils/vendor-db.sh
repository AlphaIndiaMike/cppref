#!/usr/bin/env bash
set -euo pipefail

SQLITE_VERSION="3510100"
LEVELDB_VERSION="1.23"
LMDB_VERSION="0.9.33"

TARGET_DIR="../90_vendored/database"

SQLITE_URL="https://sqlite.org/2025/sqlite-amalgamation-${SQLITE_VERSION}.zip"
LEVELDB_URL="https://github.com/google/leveldb/archive/refs/tags/${LEVELDB_VERSION}.tar.gz"
LMDB_URL="https://github.com/LMDB/lmdb/archive/refs/tags/LMDB_${LMDB_VERSION}.tar.gz"

mkdir -p "$TARGET_DIR"

# SQLite
if [[ -f "${TARGET_DIR}/sqlite3/VERSION" ]] && [[ "$(cat "${TARGET_DIR}/sqlite3/VERSION")" == "$SQLITE_VERSION" ]]; then
    echo "SQLite ${SQLITE_VERSION} already installed"
else
    echo "Installing SQLite..."
    tmp=$(mktemp -d)
    curl -fSL -o "${tmp}/sqlite.zip" "$SQLITE_URL"
    unzip -q "${tmp}/sqlite.zip" -d "$tmp"
    rm -rf "${TARGET_DIR}/sqlite3"
    mkdir -p "${TARGET_DIR}/sqlite3"
    cp "${tmp}"/sqlite-amalgamation-*/sqlite3.c "${TARGET_DIR}/sqlite3/"
    cp "${tmp}"/sqlite-amalgamation-*/sqlite3.h "${TARGET_DIR}/sqlite3/"
    [[ -f "${tmp}"/sqlite-amalgamation-*/sqlite3ext.h ]] && cp "${tmp}"/sqlite-amalgamation-*/sqlite3ext.h "${TARGET_DIR}/sqlite3/"
    echo "$SQLITE_VERSION" > "${TARGET_DIR}/sqlite3/VERSION"
    rm -rf "$tmp"
    echo "SQLite done"
fi

# LevelDB
if [[ -f "${TARGET_DIR}/leveldb/VERSION" ]] && [[ "$(cat "${TARGET_DIR}/leveldb/VERSION")" == "$LEVELDB_VERSION" ]]; then
    echo "LevelDB ${LEVELDB_VERSION} already installed"
else
    echo "Installing LevelDB..."
    tmp=$(mktemp -d)
    curl -fSL -o "${tmp}/leveldb.tar.gz" "$LEVELDB_URL"
    tar -xzf "${tmp}/leveldb.tar.gz" -C "$tmp"
    rm -rf "${TARGET_DIR}/leveldb"
    mkdir -p "${TARGET_DIR}/leveldb"
    cp -r "${tmp}/leveldb-${LEVELDB_VERSION}/"* "${TARGET_DIR}/leveldb/"
    echo "$LEVELDB_VERSION" > "${TARGET_DIR}/leveldb/VERSION"
    rm -rf "$tmp"
    echo "LevelDB done"
fi

# LMDB
if [[ -f "${TARGET_DIR}/lmdb/VERSION" ]] && [[ "$(cat "${TARGET_DIR}/lmdb/VERSION")" == "$LMDB_VERSION" ]]; then
    echo "LMDB ${LMDB_VERSION} already installed"
else
    echo "Installing LMDB..."
    tmp=$(mktemp -d)
    curl -fSL -o "${tmp}/lmdb.tar.gz" "$LMDB_URL"
    tar -xzf "${tmp}/lmdb.tar.gz" -C "$tmp"
    rm -rf "${TARGET_DIR}/lmdb"
    mkdir -p "${TARGET_DIR}/lmdb"
    cp "${tmp}/lmdb-LMDB_${LMDB_VERSION}/libraries/liblmdb/lmdb.h" "${TARGET_DIR}/lmdb/"
    cp "${tmp}/lmdb-LMDB_${LMDB_VERSION}/libraries/liblmdb/mdb.c" "${TARGET_DIR}/lmdb/"
    cp "${tmp}/lmdb-LMDB_${LMDB_VERSION}/libraries/liblmdb/midl.h" "${TARGET_DIR}/lmdb/"
    cp "${tmp}/lmdb-LMDB_${LMDB_VERSION}/libraries/liblmdb/midl.c" "${TARGET_DIR}/lmdb/"
    echo "$LMDB_VERSION" > "${TARGET_DIR}/lmdb/VERSION"
    rm -rf "$tmp"
    echo "LMDB done"
fi

echo "All done."
