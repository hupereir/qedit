Name:       @CPACK_PACKAGE_NAME@
Version:    @CPACK_PACKAGE_VERSION@
Release:    0%{dist}
Vendor: @CPACK_PACKAGE_VENDOR@

License: GPLV2

Summary: Qt based advanced text editor
Group: Applications/Editors

Source: %{name}-%{version}.tar.gz
BuildRoot: /var/tmp/%{name}-buildroot

%description
Qt based advanced text editor. Supports syntax highlighting, matching-parenthesis highlighting, auto-indentation,
customizable text macro for many languages, such as C/C++, fortran, Makefile, HTML/XML, etc. Can be plugged with aspell to provide
automatic spell-checking.

%prep
%setup -q -n %{name}-%{version} %{release}

%build
%define prefix /usr
cmake -DCMAKE_INSTALL_PREFIX=%{prefix} -DUSE_QT5=1 .
make -j4

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc COPYING INSTALL
%{prefix}/bin/qedit

%changelog
