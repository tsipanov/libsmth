Name:           libsmth
Version:        0.0.3
Release:        1%{?dist}
Summary:        Open Source implementation of SmoothStream ©

Group:          System Environment/Libraries
License:        GPLv2
URL:            http://code.google.com/p/libsmth
Source0:        libsmth-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires:       expat libcurl

%description
Libsmth aims at providing an open source implementation of Microsoft's smth©
protocol, as of the specification released on 9 August 2009.
The implementation is covered by the Microsoft Community Promise.


%package        devel
Summary:        Development files for %{name}
Group:          Development/Libraries
Requires:       %{name} = %{version}-%{release}

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%package        static
Summary:        Static build of %{name}
Group:          Development/Libraries
Requires:       %{name} = %{version}-%{release}-devel

%description    static
The %{name}-static package contains static libraries for applications that
will embed %{name}.


%prep
%setup -q


%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf $RPM_BUILD_ROOT


%post -p /sbin/ldconfig


%postun -p /sbin/ldconfig


%files
%defattr(-,root,root,-)
%doc AUTHORS BUGS TODO
%{_libdir}/*.so.*
%{_bindir}/smth-*

%files devel
%defattr(-,root,root,-)
%doc AUTHORS BUGS TODO
%{_includedir}/*.h
%{_libdir}/*.so
%{_libdir}/*.la

%files static
%defattr(-,root,root,-)
%doc AUTHORS BUGS TODO
%{_libdir}/*.a

%changelog
