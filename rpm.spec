#
# spec file for package Cybrinth
#
# Copyright (c) 2017 James Dearing <dearingj@lifetime.oregonstate.edu>
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://github.com/dearingj/Cybrinth/issues
#

#Preamble
Name:			Cybrinth
License:		AGPL-3.0+
Group:			Amusements/Games/Other
Summary:		Retro-themed 2D maze exploration game
Version:		0.1
Release:		0
Source0:		%name-%version.tar.xz
BuildRoot:		%{_tmppath}/%{name}-%{version}-build
URL:	https://github.com/dearingj/Cybrinth
Icon: images/icon.png

#Dependencies
BuildArch:	noarch
BuildRequires:	c++_compiler python gimp gimp-plugins-python irrlicht-devel boost-devel libSDL2_mixer-devel libtag-devel freetype2-devel xvfb-run automake python-xml
Provides: cybrinth
AutoReqProv: yes

%description
A retro-themed two-dimensional maze exploration game inspired by Pac-Man and similar games.

%prep
%setup
./configure

%build
%make_build

%install
%make_install

%files
%defattr(-,root,root,-)
%{_bindir}/cybrinth
%{_sysconfdir}/%{name}/
%{_datadir}/%{name}/
%config(noreplace) %{_sysconfdir}/%{name}/prefs.cfg %{_sysconfdir}/%{name}/controls.cfg
%doc AUTHORS COPYING INSTALL NEWS README credits.txt

%changelog
* Sat May 30 2015 James Dearing <dearingj@lifetime.oregonstate.edu> 0.1
- Just putting this here because I'm apparently required to enter a changelog or it won't build. For the real changelog, see the Git commit history.

