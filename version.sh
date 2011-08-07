#!/bin/bash
cd $(dirname $0)
now=$(date +%F)
revision=$(git describe --always)
version=$(echo "${now}-${revision}")
version=$(echo $(git describe --always HEAD --abbrev=0)+$(git rev-list HEAD -n 1 |cut -c 1-7))
#echo ${now}
#echo ${revision}
echo ${version}
