/* KDevelop QMake Support
 *
 * Copyright 2006-2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2008 Hamish Rodda <rodda@kde.org>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef QMAKEJOB_H
#define QMAKEJOB_H

#include <outputview/outputexecutejob.h>

#include <QProcess>

namespace KDevelop{
    class CommandExecutor;
    class IProject;
}

/**
@author Andreas Pakulat
@author Hamish Rodda (KJob porting)
*/
class QMakeJob : public KDevelop::OutputExecuteJob
{
    Q_OBJECT

public:
    explicit QMakeJob(QObject *parent = nullptr);

    enum ErrorTypes {
        NoProjectError = UserDefinedError,
        ConfigureError
    };

    void setProject(KDevelop::IProject* project);
    
    void start() override;

    QUrl workingDirectory() const override;
    QStringList commandLine() const override;

protected:
    bool doKill() override;

private Q_SLOTS:
    void slotFailed(QProcess::ProcessError);
    void slotCompleted(int);
    
private:
    KDevelop::IProject* m_project = nullptr;
    KDevelop::CommandExecutor* m_cmd = nullptr;
    bool m_killed = false;
};

#endif // QMAKEJOB_H

