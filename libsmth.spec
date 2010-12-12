Name:           libsmth
Version:        0.0.1
Release:        1%{?dist}
Summary:        Open Source implementation of SmoothStream(C)

Group:          System Environment/Libraries
License:        GPLv2
URL:            http://code.google.com/p/libsmth
Source0:        libsmth-0.0.1.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires:       libexpat libcurl

%description


%package        devel
Summary:        Development files for %{name}
Group:          Development/Libraries
Requires:       %{name} = %{version}-%{release}

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%package        static
Summary:        Development files for %{name}
Group:          Development/Libraries
Requires:       %{name} = %{version}-%{release}

%description    static
The %{name}-devel package contains static libraries applications that
will embed %{name}.


%prep
%setup -q


%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
#find $RPM_BUILD_ROOT -name '*.la' -delete


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
%{_includedir}/*
%{_libdir}/*.so

%files static
%defattr(-,root,root,-)
%doc AUTHORS BUGS TODO
%{_libdir}/*.a
%{_libdir}/*.la

%changelog
