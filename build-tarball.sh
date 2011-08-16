#!/bin/bash

git archive master |bzip2 > slowmoVideo-$(./version.sh).tar.bz2