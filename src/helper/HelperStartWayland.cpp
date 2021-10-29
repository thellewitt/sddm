/*
 * Session process wrapper
 * Copyright (C) 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
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
 *
 */

/**
 * This application sole purpose is to launch a wayland compositor (first
 * argument) and as soon as it's set up to launch a client (second argument)
 */

#include <unistd.h>
#include <QCoreApplication>
#include <QTextStream>
#include "waylandhelper.h"
#include "MessageHandler.h"
#include "HelperSignalHandler.h"

void WaylandHelperMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    SDDM::messageHandler(type, context, QStringLiteral("WaylandHelper: "), msg);
}

int main(int argc, char** argv)
{
    qInstallMessageHandler(WaylandHelperMessageHandler);
    signal(SIGTERM, sigtermHandler);

    Q_ASSERT(::getuid() != 0);
    QCoreApplication app(argc, argv);
    if (argc != 3) {
        QTextStream(stderr) << "Wrong number of arguments\n";
        return 1;
    }

    using namespace SDDM;
    WaylandHelper helper;
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &helper, [&helper] {
        qDebug("quitting helper-start-wayland");
        helper.stop();
    });
    QObject::connect(&helper, &WaylandHelper::failed, &app, [&app] {
        QTextStream(stderr) << "Failed to start wayland session" << Qt::endl;
        app.exit(2);
    });

    helper.startCompositor(app.arguments()[1]);
    helper.startGreeter(app.arguments()[2]);
    return app.exec();
}
