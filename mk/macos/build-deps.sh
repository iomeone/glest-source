#!/bin/sh
brew install sdl2 lua fontconfig freetype ftgl libogg glew libvorbis cppunit glib fribidi miniupnpc wxmac
brew install cmake
brew outdated cmake || brew upgrade cmake; brew outdated pkgconfig || brew upgrade pkgconfig
brew install Caskroom/cask/xquartz