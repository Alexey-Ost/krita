/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2010 Thomas Zander <zander@kde.org>
   Copyright (C) 2011 Inge Wallin <inge@lysator.liu.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoEmbeddedDocumentSaver.h"

#include <QList>

#include <OdfDebug.h>
#include <QUrl>

#include <KoStore.h>
#include <KoXmlWriter.h>
#include <KoOdfWriteStore.h>

#include <KoOdfManifestEntry.h>


#define INTERNAL_PROTOCOL "intern"

struct FileEntry {
    QString path;
    QByteArray mimeType;       // QBA because this is what addManifestEntry wants
    QByteArray contents;
};


class Q_DECL_HIDDEN KoEmbeddedDocumentSaver::Private
{
public:
    Private() {}

    QHash<QString, int> prefixes; // Used in getFilename();

    // These will be saved when saveEmbeddedDocuments() is called.
    QList<FileEntry*> files;    // Embedded files.
    QList<KoOdfManifestEntry*> manifestEntries;
};

KoEmbeddedDocumentSaver::KoEmbeddedDocumentSaver()
    : d(new Private())
{
}

KoEmbeddedDocumentSaver::~KoEmbeddedDocumentSaver()
{
    qDeleteAll(d->files);
    qDeleteAll(d->manifestEntries);
    delete d;
}


QString KoEmbeddedDocumentSaver::getFilename(const QString &prefix)
{
    int index = 1;
    if (d->prefixes.contains(prefix)) {
        index = d->prefixes.value(prefix);
    }

    // This inserts prefix into the map if it's not there.
    d->prefixes[prefix] = index + 1;

    //return prefix + QString("%1").arg(index, 4, 10, QChar('0'));
    return prefix + QString("%1").arg(index);
}

void KoEmbeddedDocumentSaver::saveManifestEntry(const QString &fullPath, const QString &mediaType,
                                                const QString &version)
{
    d->manifestEntries.append(new KoOdfManifestEntry(fullPath, mediaType, version));
}
