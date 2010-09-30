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
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>
#include <QDebug>
#include <QFile>
#include <QUrl>

#include "uploader.h"

Uploader::Uploader(const QString & apiKey, QObject * parent): QObject(parent), m_apiKey(apiKey)
{
    m_manager = new QNetworkAccessManager(this);

    connect(m_manager, SIGNAL(finished(QNetworkReply *)),
            this, SLOT(requestFinished(QNetworkReply *)));
}

void Uploader::uploadFile(const QString & fileName)
{
    if (fileName.isEmpty())
        return;

    // Pack the file into a base-64 encoded byte array
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        emit uploadError(tr("Unable to open image file for reading"));
        return;
    }

    QByteArray fileData = file.readAll().toBase64();
    file.close();

    // Create the request body
    QByteArray requestBody;

    requestBody.append(QString("key=").toUtf8());
    requestBody.append(QUrl::toPercentEncoding(m_apiKey));
    requestBody.append(QString("&image=").toUtf8());
    requestBody.append(QUrl::toPercentEncoding(fileData));

    // Start network access
    QNetworkReply * reply = m_manager->post(
                                QNetworkRequest(QUrl("http://api.imgur.com/2/upload.xml")),
                                requestBody);

    connect(reply, SIGNAL(uploadProgress(qint64, qint64)),
            this, SIGNAL(uploadProgress(qint64, qint64)));

}

void Uploader::requestFinished(QNetworkReply * reply)
{
    // Did we have a network error?
    if (reply->error() != QNetworkReply::NoError) {
        emit uploadError(tr("Network error: %1").arg(reply->error()));

        reply->deleteLater();
        return;
    }

    // Parse the reply
    QHash<int, QString> linksMap;
    QDomDocument doc;
    QString error;

    if (!doc.setContent(reply->readAll(), false, &error)) {
        emit uploadError(tr("Parse error: %1").arg(error));

        reply->deleteLater();
        return;
    }

    // See if we have an upload reply, or an error
    QDomElement rootElem = doc.documentElement();
    error = "";

    if (rootElem.tagName() == "upload") {
        // Skip all elements until we reach the <links> tag
        QDomNode n = rootElem.firstChild();

        while (!n.isNull()) {
            if (n.isElement() && n.toElement().tagName() == "links") {
                QDomNode innerNode = n.firstChild();

                while (!innerNode.isNull()) {
                    if (innerNode.isElement()) {
                        QDomElement e = innerNode.toElement();

                        if (e.tagName() == "original") {
                            linksMap.insert(OriginalImage, e.text());
                        }
                        else if (e.tagName() == "small_square") {
                            linksMap.insert(SmallSquare, e.text());
                        }
                        else if (e.tagName() == "large_thumbnail") {
                            linksMap.insert(LargeThumbnail, e.text());
                        }
                        else if (e.tagName() == "imgur_page") {
                            linksMap.insert(ImgurPage, e.text());
                        }
                        else if (e.tagName() == "delete_page") {
                            linksMap.insert(DeletePage, e.text());
                        }
                    }
                    innerNode = innerNode.nextSibling();
                }
            }
            n = n.nextSibling();
        }
    }
    else if (rootElem.tagName() == "error") {
        // Skip all elements until we reach the <message> tag
        QDomNode n = rootElem.firstChild();

        while (!n.isNull()) {
            if (n.isElement() && n.toElement().tagName() == "message") {
                    error = n.toElement().text();
            }
            n = n.nextSibling();
        }
    }
    else {
        error = tr("Reccived unexpected reply from web service");
    }

    if (!error.isEmpty()) {
        emit uploadError(error);
    }
    else {
        emit uploadDone(linksMap);
    }
    reply->deleteLater();
}