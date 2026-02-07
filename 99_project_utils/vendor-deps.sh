#!/usr/bin/env bash
set -euo pipefail

SQLITE_VERSION="3510100"
LEVELDB_VERSION="1.23"
LMDB_VERSION="0.9.33"

# Network / JSON
HTTPLIB_VERSION="0.30.2"
NLOHMANN_JSON_VERSION="3.12.0"

DB_DIR="../90_vendored/database"
NET_DIR="../90_vendored/network"

SQLITE_URL="https://sqlite.org/2025/sqlite-amalgamation-${SQLITE_VERSION}.zip"
LEVELDB_URL="https://github.com/google/leveldb/archive/refs/tags/${LEVELDB_VERSION}.tar.gz"
LMDB_URL="https://github.com/LMDB/lmdb/archive/refs/tags/LMDB_${LMDB_VERSION}.tar.gz"
HTTPLIB_URL="https://github.com/yhirose/cpp-httplib/archive/refs/tags/v${HTTPLIB_VERSION}.tar.gz"
NLOHMANN_JSON_URL="https://github.com/nlohmann/json/releases/download/v${NLOHMANN_JSON_VERSION}/json.hpp"

mkdir -p "$DB_DIR" "$NET_DIR"

# SQLite
if [[ -f "${DB_DIR}/sqlite3/VERSION" ]] && [[ "$(cat "${DB_DIR}/sqlite3/VERSION")" == "$SQLITE_VERSION" ]]; then
    echo "SQLite ${SQLITE_VERSION} already installed"
else
    echo "Installing SQLite..."
    tmp=$(mktemp -d)
    curl -fSL -o "${tmp}/sqlite.zip" "$SQLITE_URL"
    unzip -q "${tmp}/sqlite.zip" -d "$tmp"
    rm -rf "${DB_DIR}/sqlite3"
    mkdir -p "${DB_DIR}/sqlite3"
    cp "${tmp}"/sqlite-amalgamation-*/sqlite3.c "${DB_DIR}/sqlite3/"
    cp "${tmp}"/sqlite-amalgamation-*/sqlite3.h "${DB_DIR}/sqlite3/"
    [[ -f "${tmp}"/sqlite-amalgamation-*/sqlite3ext.h ]] && cp "${tmp}"/sqlite-amalgamation-*/sqlite3ext.h "${DB_DIR}/sqlite3/"
    echo "$SQLITE_VERSION" > "${DB_DIR}/sqlite3/VERSION"
    rm -rf "$tmp"
    echo "SQLite done"
fi

# LevelDB
if [[ -f "${DB_DIR}/leveldb/VERSION" ]] && [[ "$(cat "${DB_DIR}/leveldb/VERSION")" == "$LEVELDB_VERSION" ]]; then
    echo "LevelDB ${LEVELDB_VERSION} already installed"
else
    echo "Installing LevelDB..."
    tmp=$(mktemp -d)
    curl -fSL -o "${tmp}/leveldb.tar.gz" "$LEVELDB_URL"
    tar -xzf "${tmp}/leveldb.tar.gz" -C "$tmp"
    rm -rf "${DB_DIR}/leveldb"
    mkdir -p "${DB_DIR}/leveldb"
    cp -r "${tmp}/leveldb-${LEVELDB_VERSION}/"* "${DB_DIR}/leveldb/"
    echo "$LEVELDB_VERSION" > "${DB_DIR}/leveldb/VERSION"
    rm -rf "$tmp"
    echo "LevelDB done"
fi

# LMDB
if [[ -f "${DB_DIR}/lmdb/VERSION" ]] && [[ "$(cat "${DB_DIR}/lmdb/VERSION")" == "$LMDB_VERSION" ]]; then
    echo "LMDB ${LMDB_VERSION} already installed"
else
    echo "Installing LMDB..."
    tmp=$(mktemp -d)
    curl -fSL -o "${tmp}/lmdb.tar.gz" "$LMDB_URL"
    tar -xzf "${tmp}/lmdb.tar.gz" -C "$tmp"
    rm -rf "${DB_DIR}/lmdb"
    mkdir -p "${DB_DIR}/lmdb"
    cp "${tmp}/lmdb-LMDB_${LMDB_VERSION}/libraries/liblmdb/lmdb.h" "${DB_DIR}/lmdb/"
    cp "${tmp}/lmdb-LMDB_${LMDB_VERSION}/libraries/liblmdb/mdb.c" "${DB_DIR}/lmdb/"
    cp "${tmp}/lmdb-LMDB_${LMDB_VERSION}/libraries/liblmdb/midl.h" "${DB_DIR}/lmdb/"
    cp "${tmp}/lmdb-LMDB_${LMDB_VERSION}/libraries/liblmdb/midl.c" "${DB_DIR}/lmdb/"
    echo "$LMDB_VERSION" > "${DB_DIR}/lmdb/VERSION"
    rm -rf "$tmp"
    echo "LMDB done"
fi

# cpp-httplib (header-only, single file)
if [[ -f "${NET_DIR}/httplib/VERSION" ]] && [[ "$(cat "${NET_DIR}/httplib/VERSION")" == "$HTTPLIB_VERSION" ]]; then
    echo "cpp-httplib ${HTTPLIB_VERSION} already installed"
else
    echo "Installing cpp-httplib..."
    tmp=$(mktemp -d)
    curl -fSL -o "${tmp}/httplib.tar.gz" "$HTTPLIB_URL"
    tar -xzf "${tmp}/httplib.tar.gz" -C "$tmp"
    rm -rf "${NET_DIR}/httplib"
    mkdir -p "${NET_DIR}/httplib"
    cp "${tmp}/cpp-httplib-${HTTPLIB_VERSION}/httplib.h" "${NET_DIR}/httplib/"
    echo "$HTTPLIB_VERSION" > "${NET_DIR}/httplib/VERSION"
    rm -rf "$tmp"
    echo "cpp-httplib done"
fi

# nlohmann/json (header-only, single file)
if [[ -f "${NET_DIR}/nlohmann/VERSION" ]] && [[ "$(cat "${NET_DIR}/nlohmann/VERSION")" == "$NLOHMANN_JSON_VERSION" ]]; then
    echo "nlohmann/json ${NLOHMANN_JSON_VERSION} already installed"
else
    echo "Installing nlohmann/json..."
    rm -rf "${NET_DIR}/nlohmann"
    mkdir -p "${NET_DIR}/nlohmann"
    curl -fSL -o "${NET_DIR}/nlohmann/json.hpp" "$NLOHMANN_JSON_URL"
    echo "$NLOHMANN_JSON_VERSION" > "${NET_DIR}/nlohmann/VERSION"
    echo "nlohmann/json done"
fi

echo "All done."
