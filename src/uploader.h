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
#ifndef UPLOADER_H
#define UPLOADER_H

#include <QObject>
#include <QHash>

class QNetworkAccessManager;
class QNetworkReply;

class Uploader : public QObject
{
    Q_OBJECT
public:
    enum ImageLinks {
        OriginalImage,
        SmallSquare,
        LargeThumbnail,
        ImgurPage,
        DeletePage
    };
public:
    Uploader(const QString & apiKey, QObject * parent = 0);

    void uploadFile(const QString & fileName);

private slots:
    void requestFinished(QNetworkReply * reply);

signals:
    void uploadDone(const QHash<int, QString> &);
    void uploadError(const QString &);
    void uploadProgress(qint64, qint64);

private:
    QNetworkAccessManager * m_manager;

    QString m_apiKey;
};

#endif //UPLOADER_H