#!/usr/bin/env bash
# Build the complete server with the same Lua 5.1 dependencies on either ISA.
set -euo pipefail

repo_root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)
build_dir=${1:-"$repo_root/build/validation"}
build_jobs=${BUILD_JOBS:-2}

if [[ -n ${EXPECTED_MACHINE:-} && $(uname -m) != "$EXPECTED_MACHINE" ]]; then
  echo "Expected native $EXPECTED_MACHINE runner, found $(uname -m)" >&2
  exit 1
fi

mkdir -p "$build_dir"
cmake -S "$repo_root" -B "$build_dir" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DEQEMU_BUILD_SERVER=ON \
  -DEQEMU_BUILD_LOGIN=ON \
  -DEQEMU_BUILD_TESTS=OFF \
  -DEQEMU_BUILD_LUA=ON \
  -DEQEMU_PREFER_LUA=ON \
  -DEQEMU_BUILD_CLIENT_FILES=ON \
  2>&1 | tee "$build_dir/configure.log"
cmake --build "$build_dir" --parallel "$build_jobs" \
  2>&1 | tee "$build_dir/build.log"

server_names=(world zone loginserver eqlaunch ucs queryserv shared_memory import_client_files export_client_files)
for name in "${server_names[@]}"; do
  binary="$build_dir/bin/$name"
  test -x "$binary"
  file "$binary"
  dependencies=$(ldd "$binary")
  printf '%s\n' "$dependencies" > "$build_dir/$name.ldd.txt"
  if [[ "$dependencies" == *"not found"* ]]; then
    cat "$build_dir/$name.ldd.txt" >&2
    exit 1
  fi
done

git -C "$repo_root" rev-parse HEAD > "$build_dir/source-commit.txt"
git -C "$repo_root" status --porcelain=v1 > "$build_dir/source-status.txt"
uname -m > "$build_dir/architecture.txt"
# Preserve executable modes when downloading GitHub Actions artifacts.
archive_files=(source-commit.txt source-status.txt architecture.txt)
for name in "${server_names[@]}"; do
  archive_files+=("bin/$name")
done
tar -czf "$build_dir/server-binaries-$(uname -m).tar.gz" \
  -C "$build_dir" "${archive_files[@]}"

