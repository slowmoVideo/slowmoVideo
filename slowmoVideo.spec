Name:           slowmoVideo
Version:        0.3.1
Release:        1%{?dist}
Summary:        slowmoVideo is an OpenSource program that creates slow-motion videos from your footage.

Group:          Applications/Multimedia
License:        GPLv3
URL:            http://slowmovideo.granjow.net/
Source0:        http://slowmovideo.granjow.net/builds/%{name}-sources-v0.3+2d2b352.tar.bz2
Patch0:			%{name}-%{version}-rpm.patch
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: cmake
BuildRequires: ffmpeg-devel
BuildRequires: qt-devel
BuildRequires: gcc-c++
BuildRequires: glew-devel
BuildRequires: glut-devel
BuildRequires: SDL-devel
BuildRequires: libpng-devel
BuildRequires: libjpeg-devel
BuildRequires: opencv-devel

Requires: ffmpeg
Requires: qt
Requires: glew
Requires: glut
Requires: SDL
Requires: libpng
Requires: libjpeg
Requires: opencv


%description
slowmoVideo is an OpenSource program that creates slow-motion
videos from your footage.

%package devel
Summary: Development libraries for %{name} 
Requires: %{name}%{_isa} = %{version}-%{release}
%description devel
slowmoVideo is an OpenSource program that creates slow-motion
videos from your footage.
This package contains development files.


%prep
%setup -q -c "%{name}-%{version}
%patch0 -p1


%build
mkdir -p %{_target_platform}-1
pushd %{_target_platform}-1
%{cmake} -D CMAKE_INSTALL_PREFIX:STRING=%{_prefix} \
	-D CMAKE_BUILD_TYPE:STRING=Release \
	-D ENABLE_TESTS:BOOL=false ../slowmoVideo
popd
make %{?_smp_mflags} -C %{_target_platform}-1

mkdir -p %{_target_platform}-2
pushd %{_target_platform}-2
%{cmake} -D CMAKE_INSTALL_PREFIX:STRING=%{_prefix} \
	-D CMAKE_BUILD_TYPE:STRING=Release \
	-D BUILD_INCLUDE_DIR:STRING=../slowmoVideo/lib \
	-D BUILD_LIB_DIR:STRING=../%{_target_platform}-1/lib ../V3D
popd
make %{?_smp_mflags} -C %{_target_platform}-2


%install
rm -rf $RPM_BUILD_ROOT
make install/fast DESTDIR=$RPM_BUILD_ROOT -C %{_target_platform}-1
make install/fast DESTDIR=$RPM_BUILD_ROOT -C %{_target_platform}-2


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc README.md todo.org
%{_bindir}/slowmoFlowEdit
%{_bindir}/slowmoInfo
%{_bindir}/slowmoInterpolate
%{_bindir}/slowmoRenderer
%{_bindir}/slowmoUI
%{_bindir}/slowmoVideoInfo
%{_bindir}/slowmoVisualizeFlow
%{_bindir}/slowmoFlowBuilder
%{_libdir}/libV3D.so


%files devel
%defattr(-,root,root,-)
%{_includedir}/%{name}/flowField_sV.h
%{_includedir}/%{name}/flowRW_sV.h
%{_includedir}/%{name}/flowTools_sV.h
%{_libdir}/%{name}/libsVflow.a


%changelog
* Tue Feb  5 2013 Steven Boswell <ulatekh@yahoo.com>
  Initial .spec file
