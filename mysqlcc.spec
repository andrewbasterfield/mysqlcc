#
# spec file for mysqlcc
# (c) 2002 MySQL AB
#

%define name	mysqlcc
%define	rel	1
%global	build_redhat	%([ -e /etc/redhat-release ]; echo $[1-$?])

Name: 		%{name}
Version:	0.9.8
License:	GPL
Group:		Applications/Databases
Summary:	MySQL Control Center
URL:		http://www.ribosi.com/
BuildRoot:	%{_tmppath}/%{name}-%{version}-buildroot
BuildRequires:	qt-devel mysql-devel patch
%if %build_redhat
Source:		%{name}-%{version}-src.tar.gz
BuildRequires:	ImageMagick
Release:	fc5.%{rel}
%define 	prefix	%{_prefix}
%else
Source:		%{name}-%{version}-src.tar.gz
Release:	%{rel}
%define 	prefix	/usr
%endif

%description
mysqlcc is a platform independent graphical MySQL administration client.
It is based on Trolltech's Qt toolkit.

%prep
%setup -n %{name}-%{version}-src -q

%build
export CFLAGS="$RPM_OPT_FLAGS"
./configure --prefix=%{prefix}
%if %build_redhat
export QTDIR=%{_libdir}/qt3
%endif
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{_bindir}
mkdir -p $RPM_BUILD_ROOT%{_datadir}/%{name}/translations
install -m 755 mysqlcc $RPM_BUILD_ROOT%{prefix}/bin
install -m 644 {*.wav,syntax.txt} $RPM_BUILD_ROOT%{_datadir}/%{name}
install -m 644 translations/*.{qm,ts} \
               $RPM_BUILD_ROOT%{_datadir}/%{name}/translations

%if %build_redhat
#Menu entry
mkdir -p $RPM_BUILD_ROOT%{_menudir}
cat <<EOF > $RPM_BUILD_ROOT%{_menudir}/%{name}
?package(%{name}): \
needs="x11" \
section="Applications/Databases" \
title="MySQLCC" \
longtitle="MySQLCC" \
command="%{_bindir}/mysqlcc" needs="X11" \
icon="%{name}.png"
EOF

#Menu icons
mkdir -p %{buildroot}/{%{_miconsdir},%{_liconsdir},%{_iconsdir}}
convert xpm/applicationIcon.xpm -resize 16x16 $RPM_BUILD_ROOT%{_miconsdir}/%{name}.png
convert xpm/applicationIcon.xpm $RPM_BUILD_ROOT%{_iconsdir}/%{name}.png
convert xpm/applicationIcon.xpm -resize 48x48 $RPM_BUILD_ROOT%{_liconsdir}/%{name}.png

%post


%postun

%endif

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc Changelog.txt INSTALL.txt LICENSE.txt README.txt TODO.txt
%{_bindir}/mysqlcc
%{_datadir}/%{name}
%if %build_redhat
%{_iconsdir}/%{name}.*
%{_miconsdir}/%{name}.*
%{_liconsdir}/%{name}.*
%{_menudir}/%{name}
%endif

%changelog
* Tue May 23 2006 Jeremy French <jfrench@ribosi.com> 0.9.6fc5
- added 64 bit support
- versioned for RedHat based systems (/etc/redhat-release)
- removed patch (already applied to the source code)

* Sun Jan 19 2003 Buchan Milne <bgmilne@linux-mandrake.com> 0.8.7-1mdk
- Inline the linux path patch for all rpm builds

* Thu Nov 28 2002 Buchan Milne <bgmilne@linux-mandrake.com> 0.8.6a-1mdk
- Add conditional Mandrake support

* Fri Apr 19 2002 Lenz Grimmer <lenz@mysql.com>

- initial version

