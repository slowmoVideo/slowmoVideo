#!/bin/bash
.PHONY : tarball
tarball :
	git archive master --format=tar |bzip2 >"slowmoVideo-$(shell ./version.sh).tar.bz2"

.PHONY : release
release :
	./pack-release.sh