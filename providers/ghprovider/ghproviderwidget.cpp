/* This file is part of KDevelop
 *
 * Copyright (C) 2012-2013 Miquel Sabaté <mikisabate@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <KMessageBox>

#include <QLabel>
#include <QListView>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <KLocalizedString>

#include <ghaccount.h>
#include <ghdialog.h>
#include <ghresource.h>
#include <ghlineedit.h>
#include <ghprovidermodel.h>
#include <ghproviderwidget.h>


using namespace KDevelop;
namespace gh
{

ProviderWidget::ProviderWidget(QWidget *parent)
    : IProjectProviderWidget(parent)
{
    setLayout(new QVBoxLayout());
    m_projects = new QListView(this);
    connect(m_projects, &QListView::clicked, this, &ProviderWidget::projectIndexChanged);

    m_waiting = new QLabel(i18n("Waiting for response"), this);
    m_waiting->setAlignment(Qt::AlignCenter);
    m_waiting->hide();

    ProviderModel *model = new ProviderModel(this);
    m_projects->setModel(model);
    m_projects->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resource = new Resource(this, model);
    m_account = new Account(m_resource);
    connect(m_resource, &Resource::reposUpdated, m_waiting, &QLabel::hide);

    QHBoxLayout *topLayout = new QHBoxLayout();
    m_edit = new LineEdit(this);
    m_edit->setPlaceholderText(i18n("Search"));
    m_edit->setToolTip(i18n("You can press the Return key if you do not want to wait"));
    connect(m_edit, &LineEdit::returnPressed, this, &ProviderWidget::searchRepo);
    topLayout->addWidget(m_edit);

    m_combo = new QComboBox(this);
    m_combo->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    connect(m_combo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ProviderWidget::searchRepo);
    fillCombo();
    topLayout->addWidget(m_combo);

    QPushButton *settings = new QPushButton(QIcon::fromTheme("configure"), "", this);
    settings->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    settings->setToolTip(i18n("Click this button to configure your GitHub account"));
    connect(settings, &QPushButton::clicked, this, &ProviderWidget::showSettings);
    topLayout->addWidget(settings);

    layout()->addItem(topLayout);
    layout()->addWidget(m_waiting);
    layout()->addWidget(m_projects);
}

KDevelop::VcsJob * ProviderWidget::createWorkingCopy(const QUrl &dest)
{
    QModelIndex pos = m_projects->currentIndex();
    if (!pos.isValid())
        return nullptr;

    auto plugin = ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IBasicVersionControl", "kdevgit");
    if (!plugin) {
        KMessageBox::error(0, i18n("The Git plugin could not be loaded which is required to import a GitHub project."), i18n("GitHub Provider Error"));
        return nullptr;
    }

    QString url = pos.data(ProviderModel::VcsLocationRole).toString();
    if (m_account->validAccount())
      url = "https://" + m_account->token() + "@" + url.mid(8);
    QUrl real = QUrl(url);
    VcsLocation loc(real);

    auto vc = plugin->extension<IBasicVersionControl>();
    Q_ASSERT(vc);
    return vc->createWorkingCopy(loc, dest);
}

void ProviderWidget::fillCombo()
{
    m_combo->clear();
    m_combo->addItem(i18n("User"), 1);
    m_combo->addItem(i18n("Organization"), 3);
    if (m_account->validAccount()) {
        m_combo->addItem(m_account->name(), 0);
        m_combo->setCurrentIndex(2);
    }
    const QStringList &orgs = m_account->orgs();
    foreach (const QString &org, orgs)
        m_combo->addItem(org, 2);
}

bool ProviderWidget::isCorrect() const
{
    return m_projects->currentIndex().isValid();
}

void ProviderWidget::projectIndexChanged(const QModelIndex &currentIndex)
{
    if (currentIndex.isValid()) {
        QString name = currentIndex.data().toString();
        emit changed(name);
    }
}

void ProviderWidget::showSettings()
{
    Dialog *dialog = new Dialog(this, m_account);
    connect(dialog, &Dialog::shouldUpdate, this, &ProviderWidget::fillCombo);
    dialog->show();
}

void ProviderWidget::searchRepo()
{
    bool enabled = true;
    QString uri, text = m_edit->text();
    int idx = m_combo->itemData(m_combo->currentIndex()).toInt();

    switch (idx) {
    case 0: /* Looking for this user's repo */
        uri = "/user/repos";
        enabled = false;
        break;
    case 1: /* Looking for some user's repo */
        if (text == m_account->name())
            uri = "/user/repos";
        else
            uri = QString("/users/%1/repos").arg(text);
        break;
    case 2: /* Known organization */
        text = m_combo->currentText();
        enabled = false;
    default:/* Looking for some organization's repo. */
        uri = QString("/orgs/%1/repos").arg(text);
        break;
    }
    m_edit->setEnabled(enabled);
    m_waiting->show();
    m_resource->searchRepos(uri, m_account->token());
}

} // End of namespace gh
