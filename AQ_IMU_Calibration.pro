#
#	 This file is part of AutoQuad.
#
#	 AutoQuad is free software: you can redistribute it and/or modify
#	 it under the terms of the GNU General Public License as published by
#	 the Free Software Foundation, either version 3 of the License, or
#	 (at your option) any later version.
#
#	 AutoQuad is distributed in the hope that it will be useful,
#	 but WITHOUT ANY WARRANTY; without even the implied warranty of
#	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	 GNU General Public License for more details.
#	 You should have received a copy of the GNU General Public License
#	 along with AutoQuad.  If not, see <http://www.gnu.org/licenses/>.
#
#

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AQ_IMU_Calibration
TEMPLATE = app
BASEDIR = $${_PRO_FILE_PWD_}
TARGETDIR = $${OUT_PWD}
BUILDDIR = $${OUT_PWD}/build

OBJECTS_DIR = $${BUILDDIR}/obj
MOC_DIR = $${BUILDDIR}/moc
UI_DIR = $${BUILDDIR}/ui
RCC_DIR = $${BUILDDIR}/rcc

SOURCES += main.cpp\
        MainWindow.cpp

HEADERS += MainWindow.h \
    config.h

FORMS += MainWindow.ui

RESOURCES += resources.qrc

# Windows
win32-msvc2010|win32-msvc2012|win32-g++ {

	RC_FILE = resources/icon.rc
	CONFIG += embed_manifest_exe

	# Adjust paths to backslash
	BASEDIR ~= s,/,\\,g
	TARGETDIR ~= s,/,\\,g

	#determine name of Qt DLLs to distribute
	greaterThan(QT_MAJOR_VERSION, 4) {
		QTLIBDLLPFX = "Qt5"
		QTLIBDLLSFX = ".dll"
	} else {
		QTLIBDLLPFX = "Qt"
		QTLIBDLLSFX = "4.dll"
	}
	# target folder name depends on build type
	CONFIG(release, debug|release) {
		TARGETDIR = "$${TARGETDIR}\\release"
	}
	CONFIG(debug, debug|release) {
	  TARGETDIR = "$${TARGETDIR}\\debug"
	  QTLIBDLLSFX = "d$${QTLIBDLLSFX}"
	}

	# hack around mingw deploy wierdness
	FILEWILDCARD = "*"
	win32-g++: FILEWILDCARD = "\*"

	# Copy AQ files
	QMAKE_POST_LINK += $$quote(xcopy /D /Y "$${BASEDIR}\\bin\\aq_win_all\\$${FILEWILDCARD}" "$${TARGETDIR}\\aq\\bin" /E /I $$escape_expand(\\n\\t))

	# copy required supporting DLLs for release version
	CONFIG(release, debug|release) {
		# Qt library DLLs
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\bin\\icu*.dll" "$$TARGETDIR"$$escape_expand(\\n\\t))
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\bin\\$${QTLIBDLLPFX}Core$${QTLIBDLLSFX}" "$$TARGETDIR"$$escape_expand(\\n\\t))
		QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\bin\\$${QTLIBDLLPFX}Gui$${QTLIBDLLSFX}" "$$TARGETDIR"$$escape_expand(\\n\\t))
		greaterThan(QT_MAJOR_VERSION, 4) {
			QMAKE_POST_LINK += $$quote(xcopy /D /Y "$$(QTDIR)\\bin\\$${QTLIBDLLPFX}Widgets$${QTLIBDLLSFX}" "$$TARGETDIR"$$escape_expand(\\n\\t))
		}
		# Compiler-specific DLLs
		win32-msvc2010 {
			 QMAKE_POST_LINK += $$quote(xcopy /D /Y "\"C:\\Program Files \(x86\)\\Microsoft Visual Studio 10.0\\VC\\redist\\x86\\Microsoft.VC100.CRT\\msvc?100.dll\""  "$${TARGETDIR}\\"$$escape_expand(\\n))
		}
		win32-msvc2012 {
			 QMAKE_POST_LINK += $$quote(xcopy /D /Y "\"C:\\Program Files \(x86\)\\Microsoft Visual Studio 11.0\\VC\\redist\\x86\\Microsoft.VC110.CRT\\msvc?110.dll\""  "$${TARGETDIR}\\"$$escape_expand(\\n))
		}
		win32-g++ {
			# we need to know where MinGW lives so we can copy some DLLs from there.
			MINGW_PATH = $$(MINGW_PATH)
			isEmpty(MINGW_PATH): error("MINGW_PATH not found")
			QMAKE_POST_LINK  += $$quote(xcopy /D /Y "$${MINGW_PATH}\\bin\\libwinpthread-1.dll"  "$${TARGETDIR}"$$escape_expand(\\n\\t))
			QMAKE_POST_LINK  += $$quote(xcopy /D /Y "$${MINGW_PATH}\\bin\\libstdc++-6.dll"  "$${TARGETDIR}"$$escape_expand(\\n))
		}
	}
}

# MAC OS X
macx|macx-g++42|macx-g++|macx-llvm: {

	#QMAKE_INFO_PLIST = resources/Info.plist
	ICON = resources/app_icon.icns
	DEFINES += QT_NO_WHATSTHIS
	QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6
	QMAKE_MAC_SDK = macosx10.9

	LIBS += -lm

	# Copy AQ files
	QMAKE_POST_LINK += mkdir -p $${TARGETDIR}/$${TARGET}.app/Contents/MacOS/aq/bin
	QMAKE_POST_LINK += && cp -rf $${BASEDIR}/bin/aq_osx_all/* $${TARGETDIR}/$${TARGET}.app/Contents/MacOS/aq/bin
	QMAKE_POST_LINK += && chmod +x $${TARGETDIR}/$${TARGET}.app/Contents/MacOS/aq/bin/*

	QMAKE_POST_LINK += && $$dirname(QMAKE_QMAKE)/macdeployqt $${TARGET}.app
}

# GNU/Linux
linux-g++|linux-g++-64{

	CONFIG(debug, debug|release) {
		TARGETDIR = $${TARGETDIR}/debug
	}
	CONFIG(release, debug|release) {
		TARGETDIR = $${TARGETDIR}/release
	}
	!exists($$TARGETDIR) {
		QMAKE_POST_LINK += mkdir -p $${TARGETDIR} &&
	}
	DESTDIR = $$TARGETDIR

	# Copy AQ files
	QMAKE_POST_LINK += mkdir -p $${TARGETDIR}/aq/bin
	QMAKE_POST_LINK += && cp -rf $${BASEDIR}/bin/aq_unix_all/* $${TARGETDIR}/aq/bin
	QMAKE_POST_LINK += && chmod +x $${TARGETDIR}/aq/bin/*

	linux-g++ {
		QMAKE_POST_LINK += && mv -f $${TARGETDIR}/aq/bin/cal_32 $${TARGETDIR}/aq/bin/cal
		QMAKE_POST_LINK += && mv -f $${TARGETDIR}/aq/bin/sim3_32 $${TARGETDIR}/aq/bin/sim3
		QMAKE_POST_LINK += && mv -f $${TARGETDIR}/aq/bin/cal_AIMU_32 $${TARGETDIR}/aq/bin/cal_AIMU
		QMAKE_POST_LINK += && mv -f $${TARGETDIR}/aq/bin/sim3_AIMU_32 $${TARGETDIR}/aq/bin/sim3_AIMU
	}
	linux-g++-64 {
		QMAKE_POST_LINK += && mv -f $${TARGETDIR}/aq/bin/cal_64 $${TARGETDIR}/aq/bin/cal
		QMAKE_POST_LINK += && mv -f $${TARGETDIR}/aq/bin/sim3_64 $${TARGETDIR}/aq/bin/sim3
		QMAKE_POST_LINK += && mv -f $${TARGETDIR}/aq/bin/cal_AIMU_64 $${TARGETDIR}/aq/bin/cal_AIMU
		QMAKE_POST_LINK += && mv -f $${TARGETDIR}/aq/bin/sim3_AIMU_64 $${TARGETDIR}/aq/bin/sim3_AIMU
	}

}
