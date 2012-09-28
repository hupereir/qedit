%define ver      @QEDIT_VERSION@
%define rel      0
%define prefix   /usr

Summary: Qt based advanced text editor
Name: qedit
Version: %{ver}
Release: %{rel}
License: GPL
Group: Applications/Editors
Source: %{name}-%{ver}.tar.gz
BuildRoot: /var/tmp/%{name}-buildroot

Requires: qt4-common >= 4.2 , libqtcore4 >= 4.2, libqtgui4 >= 4.2, aspell >= 0.60.4, libaspell15 >= 0.60.4

%description
Qt based advanced text editor. Supports syntax highlighting, matching-parenthesis highlighting, auto-indentation,
customizable text macro for many languages, such as C/C++, fortran, Makefile, HTML/XML, etc. Can be plugged with aspell to provide
automatic spell-checking.

%prep
%setup -q -n %{name}-%{ver} %{rel}

%build
cmake -DCMAKE_INSTALL_PREFIX=%{prefix} .
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
%{prefix}/bin/edit-document-classes
%changelog