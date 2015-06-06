#
# spec file for package Cybrinth
#
# Copyright (c) 2015 James Dearing <dearingj@lifetime.oregonstate.edu>
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
Source0:		%name-%version.tar.bz2 
BuildRoot:		%{_tmppath}/%{name}-%{version}-build
URL:	https://github.com/dearingj/Cybrinth

#Dependencies
BuildArch:	noarch
BuildRequires:	python gimp irrlicht-devel boost-devel libSDL2_mixer-devel libtag-devel freetype2-devel

%description
A retro-themed two-dimensional maze exploration game inspired by Pac-Man and similar games.

%prep 
%setup

%build 
%make_build

%install 
%make_install

%changelog
2015-05-29: dearingj@lifetime.oregonstate.edu
- Initial version of this spec file.

