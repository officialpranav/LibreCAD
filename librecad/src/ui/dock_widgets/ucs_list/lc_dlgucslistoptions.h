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

#ifndef LC_DLGUCSLISTOPTIONS_H
#define LC_DLGUCSLISTOPTIONS_H

#include "lc_dialog.h"

namespace Ui {
    class LC_DlgUCSListOptions;
}

class LC_UCSListOptions;

// todo - sand - The dialog is not used so far, however, later on may be used for more granual editing of UCS
class LC_DlgUCSListOptions : public LC_Dialog{
    Q_OBJECT
public:
    explicit LC_DlgUCSListOptions(LC_UCSListOptions* options, QWidget *parent = nullptr);
    ~LC_DlgUCSListOptions() override;
public slots:
    void validate();
protected slots:
    void languageChange();
private:
    Ui::LC_DlgUCSListOptions *ui;
    LC_UCSListOptions* m_options;
};

#endif // LC_DLGUCSLISTOPTIONS_H
