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
#ifndef QG_LAYERDIALOG_H
#define QG_LAYERDIALOG_H

#include "ui_qg_layerdialog.h"
#include "lc_dialog.h"

class RS_Layer;
class RS_LayerList;
class QG_LayerDialog : public LC_Dialog, public Ui::QG_LayerDialog{
    Q_OBJECT
public:
    QG_LayerDialog(QWidget* parent = nullptr, QString name = 0);
	~QG_LayerDialog() override;
public slots:
    void setLayer( RS_Layer * l );
    void updateLayer();
    void validate();
    void setLayerList( RS_LayerList * ll );
    void setEditLayer( bool el );
    //! @return a reference to the QLineEdit object.
    QLineEdit* getQLineEdit ();
protected:
    RS_Layer* m_layer;
    RS_LayerList* m_layerList;
    QString m_layerName;
    bool m_editLayer;
protected slots:
    void languageChange();
private:
    void init();
};

#endif // QG_LAYERDIALOG_H
