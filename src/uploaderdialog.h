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
#ifndef UPLOADERDIALOG_H
#define UPLOADERDIALOG_H

#include <QDialog>

class Uploader;

class QProgressBar;
class QPushButton;
class QGroupBox;
class QLineEdit;
class QLabel;

class UploaderDialog : public QDialog
{
    Q_OBJECT
public:
    UploaderDialog();

    bool setImageFile(const QString & file);

public slots:
    void doUpload();
    void uploadDone(const QHash<int, QString> & linksMap);
    void uploadError(const QString & error);
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void copyButtonClicked(QWidget * editWidget);

private:
    void initUI();

private:
    Uploader * m_uploader;
    QString m_fileName;

    QLabel * m_previewLabel;
    QLabel * m_fileNameLabel;
    QLabel * m_fileInfoLabel;

    QProgressBar * m_uploadProgressBar;

    QGroupBox * m_imageLinksBox;
    QLineEdit * m_originalLink;
    QLineEdit * m_smallSquareLink;
    QLineEdit * m_largeThunbnailLink;
    QLineEdit * m_imgurPageLink;
    QLineEdit * m_deletePageLink;

    QPushButton * m_uploadButton;
    QPushButton * m_closeButton;
};

#endif //UPLOADERDIALOG_H