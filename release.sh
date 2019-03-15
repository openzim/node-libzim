#!/bin/bash
git pull && \
npm version patch && \
npm publish && \
git push && \
git push --tags