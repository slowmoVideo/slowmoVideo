.PHONY : tarball
.PHONY : release

# Creates a tarball when on a git repository. (git is required.)
tarball :
	@git archive master --format=tar |bzip2 >"slowmoVideo-sources-$(shell ./version.sh).tar.bz2"
