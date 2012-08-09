/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#include "custombuildsystemconfigwidget.h"

#include <KConfig>

#include "ui_custombuildsystemconfigwidget.h"
#include "configconstants.h"
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <language/backgroundparser/parseprojectjob.h>

namespace
{

QString generateToolGroupName( CustomBuildSystemTool::ActionType type )
{
    static const int toolTypeCount = CustomBuildSystemTool::Undefined + 1;
    // Static Qt-based objects are discouraged (MSVC-incompatible), so use raw strings
    static const char* toolTypes[] = {
        "Build",
        "Configure",
        "Install",
        "Clean",
        "Prune",
        "Undefined"
    };

    Q_ASSERT( type >= 0 && type < toolTypeCount );
    return ConfigConstants::toolGroupPrefix + toolTypes[type];
}

}

CustomBuildSystemConfigWidget::CustomBuildSystemConfigWidget( QWidget* parent, KDevelop::IProject* w_project = 0 )
    : QWidget( parent ), ui( new Ui::CustomBuildSystemConfigWidget ), project( w_project )
{
    ui->setupUi( this );
    ui->configWidget->setProject(project);

    ui->addConfig->setIcon(KIcon( "list-add" ));
    ui->removeConfig->setIcon(KIcon( "list-remove" ));

    connect( ui->currentConfig, SIGNAL(activated(int)), SLOT(changeCurrentConfig(int)));
    connect( ui->configWidget, SIGNAL(changed()), SLOT(configChanged()) );

    connect( ui->addConfig, SIGNAL(clicked(bool)), SLOT(addConfig()));
    connect( ui->removeConfig, SIGNAL(clicked(bool)), SLOT(removeConfig()));
    connect( ui->currentConfig, SIGNAL(editTextChanged(QString)), SLOT(renameCurrentConfig(QString)) );

    connect( this, SIGNAL(changed()), SLOT(verify()) );
}

void CustomBuildSystemConfigWidget::loadDefaults()
{
}

void CustomBuildSystemConfigWidget::loadFrom( KConfig* cfg )
{
    ui->configWidget->clear();
    ui->currentConfig->clear();
    QStringList groupNameList;
    KConfigGroup grp = cfg->group( ConfigConstants::customBuildSystemGroup );
    foreach( const QString& grpName, grp.groupList() ) {
        KConfigGroup subgrp = grp.group( grpName );
        CustomBuildSystemConfig config;

        config.title = subgrp.readEntry( ConfigConstants::configTitleKey, "" );
        config.buildDir = subgrp.readEntry( ConfigConstants::buildDirKey, "" );

        foreach( const QString& subgrpName, subgrp.groupList() ) {
            if( subgrpName.startsWith( ConfigConstants::toolGroupPrefix ) ) {
                KConfigGroup toolgrp = subgrp.group( subgrpName );
                CustomBuildSystemTool tool;
                tool.arguments = toolgrp.readEntry( ConfigConstants::toolArguments, "" );
                tool.executable = toolgrp.readEntry( ConfigConstants::toolExecutable, KUrl() );
                tool.envGrp = toolgrp.readEntry( ConfigConstants::toolEnvironment, "default" );
                tool.enabled = toolgrp.readEntry( ConfigConstants::toolEnabled, false );
                tool.type = CustomBuildSystemTool::ActionType( toolgrp.readEntry( ConfigConstants::toolType, 0 ) );
                config.tools[tool.type] = tool;
            } else if( subgrpName.startsWith( ConfigConstants::projectPathPrefix ) ) {
                KConfigGroup pathgrp = subgrp.group( subgrpName );
                CustomBuildSystemProjectPathConfig path;
                path.path = pathgrp.readEntry( ConfigConstants::projectPathKey, "" );
                {
                    QByteArray tmp = pathgrp.readEntry( ConfigConstants::definesKey, QByteArray() );
                    QDataStream s(tmp);
                    s.setVersion( QDataStream::Qt_4_5 );
                    s >> path.defines;
                }

                {
                    QByteArray tmp = pathgrp.readEntry( ConfigConstants::includesKey, QByteArray() );
                    QDataStream s(tmp);
                    s.setVersion( QDataStream::Qt_4_5 );
                    s >> path.includes;
                }
                config.projectPaths << path;
            }
        }
        configs << config;
        ui->currentConfig->addItem( config.title );
        groupNameList << grpName;
    }
    int idx = groupNameList.indexOf( grp.readEntry( ConfigConstants::currentConfigKey, "" ) );
    if( !groupNameList.isEmpty() && idx < 0 )
        idx = 0;

    ui->currentConfig->setCurrentIndex( idx );
    changeCurrentConfig( idx );
}

void CustomBuildSystemConfigWidget::saveConfig( KConfigGroup& grp, CustomBuildSystemConfig& c, int index )
{
    // Generate group name, access and clear it
    KConfigGroup subgrp = grp.group( ConfigConstants::buildConfigPrefix + QString::number(index) );
    subgrp.deleteGroup();

    // Write current configuration key, if our group is current
    if( ui->currentConfig->currentIndex() == index )
        grp.writeEntry( ConfigConstants::currentConfigKey, subgrp.name() );

    subgrp.writeEntry( ConfigConstants::configTitleKey, c.title );
    subgrp.writeEntry( ConfigConstants::buildDirKey, c.buildDir );
    foreach( const CustomBuildSystemTool& tool, c.tools ) {
        KConfigGroup toolgrp = subgrp.group( generateToolGroupName( tool.type ) );
        toolgrp.writeEntry( ConfigConstants::toolType, int(tool.type) );
        toolgrp.writeEntry( ConfigConstants::toolEnvironment , tool.envGrp );
        toolgrp.writeEntry( ConfigConstants::toolEnabled, tool.enabled );
        toolgrp.writeEntry( ConfigConstants::toolExecutable, tool.executable );
        toolgrp.writeEntry( ConfigConstants::toolArguments, tool.arguments );
    }

    // Write paths
    int pathIndex = 0;
    foreach (const CustomBuildSystemProjectPathConfig& path, c.projectPaths) {
        KConfigGroup pathgrp = subgrp.group( ConfigConstants::projectPathPrefix + QString::number(pathIndex++) );
        pathgrp.writeEntry( ConfigConstants::projectPathKey, path.path );
        {
            QByteArray tmp;
            QDataStream s(&tmp, QIODevice::WriteOnly);
            s.setVersion( QDataStream::Qt_4_5 );
            s << path.includes;
            pathgrp.writeEntry( ConfigConstants::includesKey, tmp );
        }
        {
            QByteArray tmp;
            QDataStream s(&tmp, QIODevice::WriteOnly);
            s.setVersion( QDataStream::Qt_4_5 );
            s << path.defines;
            pathgrp.writeEntry( ConfigConstants::definesKey, tmp );
        }
    }
}

void CustomBuildSystemConfigWidget::saveTo( KConfig* cfg, KDevelop::IProject* project )
{
    KConfigGroup subgrp = cfg->group( ConfigConstants::customBuildSystemGroup );
    subgrp.deleteGroup();
    for( int i = 0; i < ui->currentConfig->count(); i++ ) {
        configs[i].title = ui->currentConfig->itemText(i);
        saveConfig( subgrp, configs[i], i );
    }
    cfg->sync();

    if ( KDevelop::IProjectController::parseAllProjectSources()) {
        KJob* parseProjectJob = new KDevelop::ParseProjectJob(project);
        KDevelop::ICore::self()->runController()->registerJob(parseProjectJob);
    }
}

void CustomBuildSystemConfigWidget::configChanged()
{
    int idx = ui->currentConfig->currentIndex();
    if( idx >= 0 && idx < configs.count() ) {
        configs[idx] = ui->configWidget->config();
        emit changed();
    }
}

void CustomBuildSystemConfigWidget::changeCurrentConfig( int idx )
{
    if( idx < 0 || idx >= configs.size() ) {
        ui->configWidget->clear();
        emit changed();
        return;
    }
    CustomBuildSystemConfig cfg = configs.at( idx );
    ui->configWidget->loadConfig( cfg );
    emit changed();
}

void CustomBuildSystemConfigWidget::addConfig()
{
    CustomBuildSystemConfig c;
    c.title = "";
    configs.append( c );
    ui->currentConfig->addItem( c.title );
    ui->currentConfig->setCurrentIndex( ui->currentConfig->count() - 1 );
    changeCurrentConfig( ui->currentConfig->currentIndex() );
}

void CustomBuildSystemConfigWidget::removeConfig()
{
    int curidx = ui->currentConfig->currentIndex();
    Q_ASSERT( curidx < configs.length() && curidx >= 0 );
    configs.removeAt( curidx );
    ui->currentConfig->removeItem( curidx );

    // Make sure the new index is valid if possible, removing the first index
    // would otherwise set the index to -1 and that will cause havoc in changeCurrentConfig
    // since it clears the config if a negative index comes in. I'm not sure that is actually
    // correct, but well easier to fix and understand here right now and the combobox should
    // be dropped ultimately anyway I guess - or at least not be editable anymore.
    int newidx = curidx > 0 ? curidx - 1 : 0;
    ui->currentConfig->setCurrentIndex( newidx );
    changeCurrentConfig( ui->currentConfig->currentIndex() );
}

void CustomBuildSystemConfigWidget::verify() {
    Q_ASSERT( ui->currentConfig->count() == configs.count() );

    bool hasAnyConfigurations = (configs.count() != 0);
    bool configurationSelected = (ui->currentConfig->currentIndex() >= 0);
    Q_ASSERT( !hasAnyConfigurations || configurationSelected );

    ui->configWidget->setEnabled( hasAnyConfigurations );
    ui->removeConfig->setEnabled( hasAnyConfigurations );
    ui->currentConfig->setEditable( hasAnyConfigurations );
}

void CustomBuildSystemConfigWidget::renameCurrentConfig(const QString& name) {
    int idx = ui->currentConfig->currentIndex();
    if( idx >= 0 && idx < configs.count() ) {
        ui->currentConfig->setItemText( idx, name );
        emit changed();
    }
}


#include "custombuildsystemconfigwidget.moc"

