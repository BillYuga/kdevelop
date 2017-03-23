/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>
    Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>

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
    Boston, MA 02110-1301, USA.
*/

#include "qthelpprovider.h"

#include <QHelpIndexModel>
#include <QHelpContentModel>

#include <QStandardPaths>

#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/parsingenvironment.h>

#include "qthelpdocumentation.h"
#include "debug.h"

using namespace KDevelop;

QtHelpProviderAbstract::QtHelpProviderAbstract(QObject *parent, const QString &collectionFileName, const QVariantList &args)
    : QObject(parent)
    , m_engine(QStandardPaths::writableLocation(QStandardPaths::DataLocation)+'/'+collectionFileName)
{
    Q_UNUSED(args);
    if( !m_engine.setupData() ) {
        qWarning() << "Couldn't setup QtHelp Collection file";
    }
}


QtHelpProviderAbstract::~QtHelpProviderAbstract()
{
}

IDocumentation::Ptr QtHelpProviderAbstract::documentationForDeclaration(Declaration* dec) const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProviderAbstract*>(this);
    if (dec) {
        static const IndexedString qmlJs("QML/JS");
        QString id;

        {
            DUChainReadLocker lock;
            id = dec->qualifiedIdentifier().toString(RemoveTemplateInformation);
            if (dec->topContext()->parsingEnvironmentFile()->language() == qmlJs && !id.isEmpty())
                id = QLatin1String("QML.") + id;
        }

        if (!id.isEmpty()) {
            QMap<QString, QUrl> links = m_engine.linksForIdentifier(id);

            if(!links.isEmpty())
                return IDocumentation::Ptr(new QtHelpDocumentation(id, links));
        }
    }

    return {};
}

QAbstractListModel* QtHelpProviderAbstract::indexModel() const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProviderAbstract*>(this);
    return m_engine.indexModel();
}

IDocumentation::Ptr QtHelpProviderAbstract::documentationForIndex(const QModelIndex& idx) const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProviderAbstract*>(this);
    QString name=idx.data(Qt::DisplayRole).toString();
    return IDocumentation::Ptr(new QtHelpDocumentation(name, m_engine.indexModel()->linksForKeyword(name)));
}

void QtHelpProviderAbstract::jumpedTo(const QUrl& newUrl) const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProviderAbstract*>(this);
    QMap<QString, QUrl> info;
    info.insert(newUrl.toString(), newUrl);
    IDocumentation::Ptr doc(new QtHelpDocumentation(newUrl.toString(), info));
    emit addHistory(doc);
}

IDocumentation::Ptr QtHelpProviderAbstract::homePage() const
{
    QtHelpDocumentation::s_provider = const_cast<QtHelpProviderAbstract*>(this);
    return IDocumentation::Ptr(new HomeDocumentation);
}

bool QtHelpProviderAbstract::isValid() const
{
    return !m_engine.registeredDocumentations().isEmpty();
}
