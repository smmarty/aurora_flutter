%global __provides_exclude_from ^%{_datadir}/%{name}/lib/.*$
%global __requires_exclude ^lib(dconf|flutter-embedder|maliit-glib|.+_platform_plugin)\\.so.*$

Name: com.example.aurora_push_example
Summary: Demonstrates how to use the aurora_push plugin.
Version: 0.2.0
Release: 2
License: Proprietary
Source0: %{name}-%{version}.tar.zst

BuildRequires: cmake
BuildRequires: pkgconfig(flutter-embedder)
BuildRequires: pkgconfig(auroraapp)
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(nemonotifications-qt5)
BuildRequires: pkgconfig(pushclient) >= 2.0.0
Requires: sailfishsilica-qt5 >= 0.10.9
Requires: nemo-qml-plugin-notifications-qt5
%description
%{summary}.

%prep
%autosetup

%build
%cmake -DCMAKE_BUILD_TYPE=%{_flutter_build_type}
%make_build

%install
%make_install

%files
%{_bindir}/%{name}
%{_datadir}/%{name}/*
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
