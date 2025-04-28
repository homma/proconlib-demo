#!/bin/sh -x

DATE=$(date '+%Y/%m/%d %H:%M')
MESSAGE="committed on ${DATE}."
ROOT_DIR=$(dirname $0)/..

cd ${ROOT_DIR}

git pull
git checkout main
git add .
git commit -a -m "${MESSAGE}"
git push -u origin main
