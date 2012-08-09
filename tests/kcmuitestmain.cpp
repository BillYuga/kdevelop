/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2012 Andreas Pakulat <apaku@gmx.de>                        *
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

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdialog.h>
#include <ktempdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <kpushbutton.h>

#include <tests/testproject.h>

#include "custombuildsystemconfigwidget.h"

static const char description[] =
    I18N_NOOP("CustomBuildSystem Config Ui Test App");

static const char version[] = "0.1";

class State : public QObject
{
Q_OBJECT
public:
    State( KDialog* dlg, CustomBuildSystemConfigWidget* cfgWidget, KConfig* config, KDevelop::IProject* proj )
        : dialog(dlg), configWidget(cfgWidget), cfg(config), project(proj)
    {
        connect(dlg, SIGNAL(applyClicked()), SLOT(apply()));
        connect(dlg, SIGNAL(okClicked()), SLOT(ok()));
        connect(dlg, SIGNAL(cancelClicked()), qApp, SLOT(quit()));
        connect(configWidget, SIGNAL(changed()), SLOT(configChanged()));
    }
public slots:
    void apply() {
        configWidget->saveTo(cfg, project);
        dialog->button( KDialog::Apply )->setEnabled(false);
        dialog->button( KDialog::Ok )->setEnabled(false);
    }
    void ok() {
        apply();
        qApp->quit();
    }
    void configChanged() {
        dialog->button( KDialog::Apply )->setEnabled(true);
        dialog->button( KDialog::Ok )->setEnabled(true);
    }
private:
    KDialog* dialog;
    CustomBuildSystemConfigWidget* configWidget;
    KConfig* cfg;
    KDevelop::IProject* project;
};

int main(int argc, char **argv)
{
    KAboutData about("kcm_uitest", 0, ki18n("kcm_uitest"), version, ki18n(description),
                     KAboutData::License_GPL, ki18n("(C) 2012 Andreas Pakulat"), KLocalizedString(), 0, "apaku@gmx.de");
    about.addAuthor( ki18n("Andreas Pakulat"), KLocalizedString(), "apaku@gmx.de" );
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    KTempDir tempdir("kdev-custom-uitest");

    KConfig projkcfg( tempdir.name() + "/kdev-custom-uitest.kdev4" );

    KDialog dlg;
    dlg.setButtons( KDialog::Ok | KDialog::Apply | KDialog::Cancel );

    KDevelop::TestProject proj;
    proj.set_projectFileUrl( KUrl(tempdir.name()));
    
    CustomBuildSystemConfigWidget widget(0, &proj);
    widget.loadFrom(&projkcfg);
    dlg.setMainWidget(&widget);

    dlg.setCaption("Ui Test App for Config Widget");

    dlg.button( KDialog::Apply )->setEnabled(false);
    dlg.button( KDialog::Ok )->setEnabled(false);

    State state(&dlg, &widget, &projkcfg, &proj );

    dlg.resize(800, 600);
    
    dlg.show();
    
    return app.exec();

}

#include "main.moc"