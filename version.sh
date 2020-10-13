#!/bin/bash

BASE=$(git describe --tags `git rev-list --tags --max-count=1`)
VERSION=`git rev-parse --short HEAD | tr -d '\n'`
echo $BASE.$VERSION