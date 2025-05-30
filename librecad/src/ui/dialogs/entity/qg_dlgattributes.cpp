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
#include "qg_dlgattributes.h"

#include "rs_modification.h"

/*
 *  Constructs a QG_DlgAttributes as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
QG_DlgAttributes::QG_DlgAttributes(QWidget* parent)
    : LC_Dialog(parent, "EntityAttributes"){
    setupUi(this);
}

/*
 *  Destroys the object and frees any allocated resources
 */
QG_DlgAttributes::~QG_DlgAttributes(){
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void QG_DlgAttributes::languageChange(){
    retranslateUi(this);
}

void QG_DlgAttributes::setData(RS_AttributesData* data, RS_LayerList& layerList) {
    m_attributesData = data;

    //pen = line->getPen();
    wPen->setPen(m_attributesData->pen, true, true, tr("Pen"));

    //RS_Graphic* graphic = line->getGraphic();
    //if (graphic) {
        cbLayer->init(layerList, false, true);
    //}
    //cbLayer->setLayer(data->layer);
    //RS_Layer* lay = line->getLayer(false);
    //if (lay) {
    //    cbLayer->setLayer(*lay);
    //}
}

void QG_DlgAttributes::updateData() {
    m_attributesData->pen = wPen->getPen();
    m_attributesData->layer = cbLayer->currentText();
    m_attributesData->changeColor = !wPen->isColorUnchanged();
    m_attributesData->changeLineType = !wPen->isLineTypeUnchanged();
    m_attributesData->changeWidth = !wPen->isWidthUnchanged();
    m_attributesData->changeLayer = !cbLayer->isUnchanged();
    m_attributesData->applyBlockDeep = cbBlockDeep->isChecked();
}
