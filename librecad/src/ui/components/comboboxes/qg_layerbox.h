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

#ifndef QG_LAYERBOX_H
#define QG_LAYERBOX_H

#include <QComboBox>

class RS_Layer;
class RS_LayerList;

/**
 * A combobox for choosing a layer.
 */
class QG_LayerBox: public QComboBox {
    Q_OBJECT
public:
    QG_LayerBox(QWidget* parent=nullptr);
    ~QG_LayerBox() override;
    RS_Layer* getLayer() {
        return m_currentLayer;
    }
    void setLayer(RS_Layer& layer);
    void setLayer(QString& layer);
    void init(RS_LayerList& ll, bool doShowByBlock, bool doShowUnchanged);
	bool isUnchanged() {
		return m_unchanged;
	}
private slots:
    void slotLayerChanged(int index);
signals:
    void layerChanged(RS_Layer* layer);
private:
    RS_LayerList* m_layerList = nullptr;
    RS_Layer* m_currentLayer = nullptr;
    bool m_showByBlock = false;
    bool m_showUnchanged = false;
    bool m_unchanged = false;
};

#endif
