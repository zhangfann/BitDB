#!/bin/bash
cd `dirname $0`

# MSG只能是单独字符, 不能有空格
MSG=$*
if [ "${MSG}"X == ""X ] ; then
    MSG="backup, shell routine backup"
fi
#git pull origin master
git add -A
git commit -m "${MSG}"
git push -u origin master
