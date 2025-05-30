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

#ifndef QG_PENTOOLBAR_H
#define QG_PENTOOLBAR_H
#include <QToolBar>

#include "lc_graphicviewaware.h"
#include "rs_layerlistlistener.h"
#include "rs_pen.h"

class RS_LayerList;
class QG_ColorBox;
class QG_WidthBox;
class QG_LineTypeBox;

/**
 * A toolbar that offers all widgets for choosing a pen.
 */
class QG_PenToolBar: public QToolBar, public LC_GraphicViewAware, public RS_LayerListListener {
    Q_OBJECT
public:
	QG_PenToolBar( const QString & title, QWidget * parent = 0 );
    ~QG_PenToolBar() override;
    void updateByLayer(RS_Layer* l);

    RS_Pen getPen() const;

    // Methods from RS_LayerListListener Interface:
    void layerActivated(RS_Layer*) override;
    void layerEdited(RS_Layer*) override;

    void setLayerColor(RS_Color color, bool b);
    void setLayerWidth(RS2::LineWidth width, bool b);
    void setLayerLineType(RS2::LineType lineType, bool b);

    void setColor(RS_Color color);
    void setWidth(RS2::LineWidth width);
    void setLineType(RS2::LineType lineType);
    void emitPenChanged();

    void setGraphicView(RS_GraphicView* gview) override;
public slots:
    void slotColorChanged(const RS_Color& color);
    void slotWidthChanged(RS2::LineWidth w);
    void slotLineTypeChanged(RS2::LineType w);
signals:
    void penChanged(RS_Pen);
private:
    void setLayerList(RS_LayerList* ll);
    RS_LayerList* m_layerList{nullptr};
	std::unique_ptr<RS_Pen> m_currentPen;
	std::unique_ptr<QG_ColorBox> m_colorBox;
	std::unique_ptr<QG_WidthBox> m_widthBox;
	std::unique_ptr<QG_LineTypeBox> m_lineTypeBox;
};

#endif
