#!/bin/bash

# Creates a tarball when on a git repository. (git is required.)
git archive master --format=tar |bzip2 >"slowmoVideo-sources-$(shell ./version.sh).tar.bz2"

