#!/bin/sh -x

ROOT_DIR=$(dirname $0)/..

cd ${ROOT_DIR}

REPO_NAME=$(basename ${PWD})

git init
git remote add origin https://github.com/homma/${REPO_NAME}.git
