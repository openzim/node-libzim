#!/bin/bash

shopt -s lastpipe

rm dev/out/* 2>/dev/null
mkdir -p dev/out

npm publish --dry-run |& sed -n '/=== Tarball Contents ===/,/=== Tarball Details ===/p' | head -n-1 | tail -n+2 | cut -c20- | \
  while read name; do
    cp --parents ${name} dev/out/
  done

echo "Done. Use 'dev/out' directory for testing"
