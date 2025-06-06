/*******************************************************************************
*
 This file is part of the LibreCAD project, a 2D CAD program

 Copyright (C) 2025 LibreCAD.org
 Copyright (C) 2025 sand1024

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/
#include "lc_customstylehelper.h"

#include <QApplication>
#include <QFile>
#include <QStyleFactory>

#include "rs_settings.h"

void LC_CustomStyleHelper::loadFromSettings() {
    LC_GROUP("Widgets");
    {
        bool allow_style = LC_GET_BOOL("AllowStyle", false);
        if (allow_style) {
            QString style = LC_GET_STR("Style", "");
            QApplication::setStyle(QStyleFactory::create(style));
        }

        QString sheet_path = LC_GET_STR("StyleSheet", "");
        if (loadStyleSheet(sheet_path)) {
            m_styleSheetPath = sheet_path;
        }
    }
}

bool LC_CustomStyleHelper::loadStyleSheet(const QString& path) {
    if (!path.isEmpty() && QFile::exists(path)) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qApp->setStyleSheet(QString::fromLatin1(file.readAll()));
            m_styleSheetPath = path;
            return true;
        }
    }
    else {
        qApp->setStyleSheet("");
        m_styleSheetPath = path;
    }
    return false;
}

void LC_CustomStyleHelper::reloadStyleSheet() {
    loadStyleSheet(m_styleSheetPath);
}
