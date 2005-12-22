/* KDevelop Automake Support
 *
 * Copyright (C)  2005  Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "automakeimporter.h"

#include <kgenericfactory.h>
#include "kdevproject.h"
#include "kdevprojectmodel.h"

#include "automakeprojectmodel.h"

K_EXPORT_COMPONENT_FACTORY( libkdevautomakeimporter,
                            KGenericFactory<AutoMakeImporter>( "kdevautomakeimporter" ) )

AutoMakeImporter::AutoMakeImporter( QObject* parent, const char* name,
                                    const QStringList& )
: KDevProjectEditor( parent )
, m_rootItem(0L)
{
	setObjectName( QString::fromUtf8( name ) );
	m_project = qobject_cast<KDevProject*>( parent );
	Q_ASSERT( m_project );
}

AutoMakeImporter::~AutoMakeImporter()
{
	//delete m_rootItem;
}

KDevProject* AutoMakeImporter::project() const
{
	return m_project;
}

KDevProjectEditor* AutoMakeImporter::editor() const
{
	return const_cast<AutoMakeImporter*>( this );
}

QList<KDevProjectFolderItem*> AutoMakeImporter::parse( KDevProjectFolderItem* dom )
{
	Q_UNUSED( dom );
	return QList<KDevProjectFolderItem*>();
}

KDevProjectItem* AutoMakeImporter::import( KDevProjectModel* model,
                                           const QString& fileName )
{
	Q_UNUSED( model );
	m_rootItem = new AutoMakeDirItem( fileName, 0 );
	return m_rootItem;	
}

QString AutoMakeImporter::findMakefile( KDevProjectFolderItem* dom ) const
{
	Q_UNUSED( dom );
	return QString();
}

QStringList AutoMakeImporter::findMakefiles( KDevProjectFolderItem* dom ) const
{
	Q_UNUSED( dom );
	return QStringList();
}

#include "automakeimporter.h"
// kate: indent-mode csands; space-indent off; tab-width 4; auto-insert-doxygen on;


