/*
 * Copyright 2010 Igor Khanin <igorkh [AT] freeshell [DOT] org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QFileInfo>
#include <QIcon>

#include <iostream>

#include "uploaderdialog.h"

int main(int argc, char ** argv)
{
    QApplication app(argc, argv);

    app.setWindowIcon(QIcon(":/imgur.png"));

    //
    QStringList args = app.arguments();
    QString file;

    if (args.size() < 2) {
        // No file provided on command line - show file dialog
        file = QFileDialog::getOpenFileName(NULL,
                                app.tr("imgur Uploader"),
                                QDir::homePath(),
                                app.tr("Images (*.jpg *.jpeg *.png *.apng *.tiff *.bmp *.gif)"));

        if (file.isEmpty()) {
            return -1;
        }
    }
    else {
        file = args.at(1);
    }

    //
    UploaderDialog * dlg = new UploaderDialog();

    if (dlg->setImageFile(file)) {
        dlg->show();
    }
    else {
        QString fileName = QFileInfo(file).fileName();
        QString errorMsg = app.tr("The file %1 does not exist, is not readable, or is "
                                  "not an image file in a format supported by imgur").arg(fileName);

        QMessageBox::information(NULL, app.tr("imgur Uploader"), errorMsg);
        return -2;
    }

    return app.exec();
}