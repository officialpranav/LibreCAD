/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software 
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!  
**
**********************************************************************/
#ifndef QG_BLOCKDIALOG_H
#define QG_BLOCKDIALOG_H

#include "ui_qg_blockdialog.h"
#include "lc_dialog.h"

class RS_BlockList;
struct RS_BlockData;

class QG_BlockDialog : public LC_Dialog, public Ui::QG_BlockDialog{
    Q_OBJECT
public:
    QG_BlockDialog(QWidget* parent = nullptr);
	~QG_BlockDialog() override =default;
    RS_BlockData getBlockData();
public slots:
    void setBlockList( RS_BlockList * l );
    void validate();
    void cancel();
protected:
    RS_BlockList* m_blockList;
protected slots:
    void languageChange();
};

#endif // QG_BLOCKDIALOG_H
