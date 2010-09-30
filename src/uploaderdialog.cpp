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
#include <QSignalMapper>
#include <QApplication>
#include <QImageReader>
#include <QProgressBar>
#include <QMessageBox>
#include <QPushButton>
#include <QGridLayout>
#include <QBoxLayout>
#include <QClipboard>
#include <QGroupBox>
#include <QFileInfo>
#include <QLineEdit>
#include <QLabel>

#include "uploaderdialog.h"
#include "uploader.h"
#include "apikey.h"

UploaderDialog::UploaderDialog()
{
    initUI();

    // Create uploader instance (API_KEY is in apikey.h)
    m_uploader = new Uploader(API_KEY, this);

    connect(m_uploader, SIGNAL(uploadDone(const QHash<int, QString> &)),
            this, SLOT(uploadDone(const QHash<int, QString> &)));

    connect(m_uploader, SIGNAL(uploadError(const QString &)),
            this, SLOT(uploadError(const QString &)));

    connect(m_uploader, SIGNAL(uploadProgress(qint64, qint64)),
            this, SLOT(uploadProgress(qint64, qint64)));
}

/*!
 *
 * @return <i>true</i> if the image file was accepted, and the dialog should be
 *         shown to the user, or <i>false</i> otherwise.
 */
bool UploaderDialog::setImageFile(const QString & file)
{
    // Attempt opening the provided file name using a QImageReader. This
    // will check if the file exists and is readable, and if its an image
    // file
    QImageReader reader;
    reader.setFileName(file);

    if (!reader.canRead()) {
        reader.device()->close();
        return false;
    }

    // Make sure the format is OK with imgur
    QStringList formats = QStringList() << "jpg" << "jpeg" << "gif"
                                        << "png" << "apng" << "tiff"
                                        << "bmp";

    QString format = QString::fromUtf8(reader.format());
    if (!formats.contains(format)) {
        reader.device()->close();
        return false;
    }

    // Read the image
    QPixmap thumbnail = QPixmap::fromImage(reader.read()).scaled(
                                    QSize(132, 100),
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);

    if (thumbnail.isNull()) {
        reader.device()->close();
        return false;
    }

    m_fileName = file;

    // Create human friendly size
    QFileInfo fileInfo(file);

    qint64 size = fileInfo.size();
    QString fileSize;

    if (size >= 1000000) {
        fileSize = tr("%1 MB").arg(size / 1000000.0);
    }
    else if (size >= 1000) {
        fileSize = tr("%1 KB").arg(size / 1000.0);
    }
    else {
        fileSize = tr("%1 B").arg(size);
    }

    // Fill the UI
    m_previewLabel->setPixmap(thumbnail);
    m_fileNameLabel->setText(fileInfo.fileName());
    m_fileInfoLabel->setText(tr("%1, %2x%3 (%4)").
                                arg(format.toUpper()).
                                arg(thumbnail.width()).
                                arg(thumbnail.height()).
                                arg(fileSize));

    m_uploadButton->setEnabled(true);

    return true;
}

void UploaderDialog::doUpload()
{
    m_uploadButton->setEnabled(false);
    m_closeButton->setEnabled(false);

    m_uploadProgressBar->setEnabled(true);
    m_uploadProgressBar->setRange(0, 100);

    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    m_uploader->uploadFile(m_fileName);
}

void UploaderDialog::uploadDone(const QHash<int, QString> & linksMap)
{
    m_uploadButton->setEnabled(false);
    m_closeButton->setEnabled(true);

    m_originalLink->setText(linksMap.value(Uploader::OriginalImage));
    m_smallSquareLink->setText(linksMap.value(Uploader::SmallSquare));
    m_largeThunbnailLink->setText(linksMap.value(Uploader::LargeThumbnail));
    m_imgurPageLink->setText(linksMap.value(Uploader::ImgurPage));
    m_deletePageLink->setText(linksMap.value(Uploader::DeletePage));

    m_imageLinksBox->setEnabled(true);
    qApp->restoreOverrideCursor();
}

void UploaderDialog::uploadError(const QString & error)
{
    QMessageBox::critical(this, tr("imgur Uploader - Error"), error);

    m_uploadButton->setEnabled(true);
    m_closeButton->setEnabled(true);
}

void UploaderDialog::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    m_uploadProgressBar->setMaximum(bytesTotal);
    m_uploadProgressBar->setValue(bytesSent);
}

void UploaderDialog::copyButtonClicked(QWidget * editWidget)
{
    QLineEdit * lineEdit = qobject_cast<QLineEdit *>(editWidget);
    if (lineEdit) {
        qApp->clipboard()->setText(lineEdit->text());
    }
}

void UploaderDialog::initUI()
{
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setMinimumWidth(560);

    setWindowTitle(tr("imgur Uploader"));

    //
    // Widgets
    //
    m_previewLabel = new QLabel();

    m_fileNameLabel = new QLabel();
    m_fileNameLabel->setStyleSheet("font-size: 17pt; font-weight: bold");

    m_fileInfoLabel = new QLabel();

    m_uploadProgressBar = new QProgressBar();
    m_uploadProgressBar->setEnabled(false);

    m_imageLinksBox = new QGroupBox(tr("Image Links"));
    m_imageLinksBox->setEnabled(false);

    m_originalLink = new QLineEdit();
    m_originalLink->setReadOnly(true);

    m_smallSquareLink = new QLineEdit();
    m_smallSquareLink->setReadOnly(true);

    m_largeThunbnailLink = new QLineEdit();
    m_largeThunbnailLink->setReadOnly(true);

    m_imgurPageLink = new QLineEdit();
    m_imgurPageLink->setReadOnly(true);

    m_deletePageLink = new QLineEdit();
    m_deletePageLink->setReadOnly(true);

    m_uploadButton = new QPushButton(tr("&Upload"));
    m_uploadButton->setEnabled(false);

    m_closeButton = new QPushButton(tr("&Close"));
    m_closeButton->setIcon(QIcon::fromTheme("window-close"));

    connect(m_uploadButton, SIGNAL(clicked()), this, SLOT(doUpload()));
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(close()));

    // Copy buttons
    QIcon copyIcon = QIcon::fromTheme("edit-copy");

    QPushButton * originalLinkCopy = new QPushButton(copyIcon, QString());
    QPushButton * smallSquareLinkCopy = new QPushButton(copyIcon, QString());
    QPushButton * largeThunbnailLinkCopy = new QPushButton(copyIcon, QString());
    QPushButton * imgurPageLinkCopy = new QPushButton(copyIcon, QString());
    QPushButton * deletePageLinkCopy = new QPushButton(copyIcon, QString());

    QSignalMapper * mapper = new QSignalMapper();
    mapper->setMapping(originalLinkCopy, m_originalLink);
    mapper->setMapping(smallSquareLinkCopy, m_smallSquareLink);
    mapper->setMapping(largeThunbnailLinkCopy, m_largeThunbnailLink);
    mapper->setMapping(imgurPageLinkCopy, m_imgurPageLink);
    mapper->setMapping(deletePageLinkCopy, m_deletePageLink);

    connect(originalLinkCopy, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(smallSquareLinkCopy, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(largeThunbnailLinkCopy, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(imgurPageLinkCopy, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(deletePageLinkCopy, SIGNAL(clicked()), mapper, SLOT(map()));

    connect(mapper, SIGNAL(mapped(QWidget *)),
            this, SLOT(copyButtonClicked(QWidget *)));

    //
    // Layout
    //
    QGridLayout * previewLayout = new QGridLayout();
    previewLayout->setColumnMinimumWidth(3, 10);
    previewLayout->setColumnStretch(4, 1);
    previewLayout->addWidget(m_previewLabel, 1, 1, 3, 2);
    previewLayout->addWidget(m_fileNameLabel, 1, 4, Qt::AlignTop | Qt::AlignLeft);
    previewLayout->addWidget(m_fileInfoLabel, 2, 4, Qt::AlignTop | Qt::AlignLeft);

    QGridLayout * imageLinksLayout = new QGridLayout(m_imageLinksBox);
    imageLinksLayout->addWidget(new QLabel(tr("Original Image:")),  1, 1);
    imageLinksLayout->addWidget(m_originalLink,                     1, 2);
    imageLinksLayout->addWidget(originalLinkCopy,                   1, 3);
    imageLinksLayout->addWidget(new QLabel(tr("Small Square:")),    2, 1);
    imageLinksLayout->addWidget(m_smallSquareLink,                  2, 2);
    imageLinksLayout->addWidget(smallSquareLinkCopy,                2, 3);
    imageLinksLayout->addWidget(new QLabel(tr("Large Thumbnail:")), 3, 1);
    imageLinksLayout->addWidget(m_largeThunbnailLink,               3, 2);
    imageLinksLayout->addWidget(largeThunbnailLinkCopy,             3, 3);
    imageLinksLayout->addWidget(new QLabel(tr("imgur Page:")),      4, 1);
    imageLinksLayout->addWidget(m_imgurPageLink,                    4, 2);
    imageLinksLayout->addWidget(imgurPageLinkCopy,                  4, 3);
    imageLinksLayout->addWidget(new QLabel(tr("Delete Page:")),     5, 1);
    imageLinksLayout->addWidget(m_deletePageLink,                   5, 2);
    imageLinksLayout->addWidget(deletePageLinkCopy,                 5, 3);

    QHBoxLayout * buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(m_uploadButton);
    buttonLayout->addWidget(m_closeButton);

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->addLayout(previewLayout);
    layout->addSpacing(5);
    layout->addWidget(m_uploadProgressBar);
    layout->addSpacing(5);
    layout->addWidget(m_imageLinksBox);
    layout->addLayout(buttonLayout);
}