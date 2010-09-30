#
# Copyright 2010 Igor Khanin <igorkh [AT] freeshell [DOT] org>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Framework Settings
TEMPLATE = app
QT += xml network

# Output Settings
TARGET = imgur-uploader

MOC_DIR = build
RCC_DIR = build
OBJECTS_DIR = build

# Input Files
DEPENDPATH += . src
INCLUDEPATH += . src

HEADERS += src/apikey.h \
           src/uploader.h \
           src/uploaderdialog.h

SOURCES += src/main.cpp \
           src/uploader.cpp \
           src/uploaderdialog.cpp

RESOURCES += icon.qrc

# Installs
unix : !macx {
    isEmpty(PREFIX): PREFIX = /usr/local

    # Application binary
    target.path = $${PREFIX}/bin

    # Icon
    icon.files = imgur.png
    icon.path = $${PREFIX}/share/pixmaps

    # Desktop file
    desktop.files = imgur-uploader.desktop
    desktop.path = $${PREFIX}/share/applications

    INSTALLS += target icon desktop

    # If KDE is installed, install service menu
    KDE_SERVICE_PATHS = $$system(kde4-config --path services | tr : \" \")

    !isEmpty(KDE_SERVICE_PATHS) {
        # Find the system-level service directory
        for(path, KDE_SERVICE_PATHS) {
            !contains(path, $$(HOME)) {
                FINAL_PATH = $${path}
            }
        }

        !isEmpty(FINAL_PATH) {
            serviceMenu.files = uploadToImgur.desktop
            serviceMenu.path = $${FINAL_PATH}/ServiceMenus

            INSTALLS += serviceMenu
        }
    }
}