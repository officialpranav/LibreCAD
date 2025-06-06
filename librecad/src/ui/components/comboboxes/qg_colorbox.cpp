/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010-2011 R. van Twisk (librecad@rvt.dds.nl)
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

#include "qg_colorbox.h"

#include <QColorDialog>
#include <QPainter>

#include "rs_color.h"
#include "rs_settings.h"

namespace
{
    constexpr size_t Max_Custom_Colors = 3;
    const QString customColorName = QObject::tr("/CustomColor%1");
    const QString customItemText = QObject::tr("Custom Picked");
    const QString userColorText = QObject::tr("User Color");

    // find total difference in colors
    int getDictionaryDistance(const RS_Color& lhs, const RS_Color& rhs) {
        return std::abs(lhs.red() - rhs.red())
            + std::abs(lhs.green() - rhs.green())
            + std::abs(lhs.blue() - rhs.blue());
    }

    void addColors(QG_ColorBox& colorBox, const std::vector<std::pair<Qt::GlobalColor, QString>>& colors) {
        for (const auto& color : colors)
            colorBox.addColor(color.first, color.second);
    }
}

/**
 * Default Constructor. You must call init manually if you choose
 * to use this constructor.
 */
QG_ColorBox::QG_ColorBox(QWidget* parent, const char* name)
    :QComboBox(parent)
    ,m_currentColor(std::make_unique<RS_Color>()){
    setObjectName(name);
    setEditable ( false );
}

/**
 * Constructor that calls init and provides a fully functional
 * combobox for choosing colors.
 *
 * @param showByLayer true: Show attribute ByLayer, ByBlock.
 */
QG_ColorBox::QG_ColorBox(bool showByLayer, bool showUnchanged,
                         QWidget* parent, const char* name)
    : QComboBox(parent)
    ,m_currentColor(new RS_Color())
    ,m_unchanged(false){

    setObjectName(name);
    setEditable ( false );
    init(showByLayer, showUnchanged);
}

QG_ColorBox::~QG_ColorBox() = default;


/**
 * Initialisation (called from constructor or manually but only
 * once).
 *
 * @param showByLayer true: Show attribute ByLayer, ByBlock.
 */
void QG_ColorBox::init(bool showByLayer, bool showUnchanged) {
    this->m_showByLayer = showByLayer;
    this->m_showUnchanged = showUnchanged;

    if (showUnchanged) {
        addItem(QIcon(":/colors/color00.png"), tr("Unchanged"));
    }
    if (showByLayer) {
        addItem(QIcon(":/colors/color00.png"), tr("By Layer"));
        addItem(QIcon(":/colors/color00.png"), tr("By Block"));
    }

    // a special "Custom" color
    addItem(QIcon(":/controls/colorxx.png"), tr("Custom"));

    //static colors starts here
    QString red(tr("Red"));
    addColor(Qt::red,red);
    m_colorIndexStart=findText(red); // keep the starting point of static colors
    addColors(*this, {
        {Qt::darkRed, tr("Dark Red")},
    });
    addColor(QColor("orange"), tr("Orange"));
    addColor(QColor(127,78,0), tr("Dark Orange"));
    addColors(*this, {
        {Qt::yellow,tr("Yellow")},
        {Qt::darkYellow,tr("Dark Yellow")},
        {Qt::green,tr("Green")},
        {Qt::darkGreen,tr("Dark Green")},
        {Qt::cyan,tr("Cyan")},
        {Qt::darkCyan,tr("Dark Cyan")},
        {Qt::blue,tr("Blue")},
        {Qt::darkBlue,tr("Dark Blue")},
        {Qt::magenta,tr("Magenta")},
        {Qt::darkMagenta,tr("Dark Magenta")},
    });

    // a special "Black/White" color
    addItem(QIcon(":/colors/color07.png"), tr("Black / White"), QColor(Qt::black));

    // Gray colors
    addColors(*this, {
        {Qt::gray,tr("Gray")},
        {Qt::darkGray,tr("Dark Gray")},
        {Qt::lightGray,tr("Light Gray")},
    });
    //static colors ends here
    // add custom colors from settings
    readCustomColorSettings();

    connect(this, &QG_ColorBox::activated,this, &QG_ColorBox::slotColorChanged);

    if (showUnchanged || showByLayer ) {
        setCurrentIndex(0);
    } else {
        setCurrentIndex(findData(QColor(Qt::black))); //default to Qt::black
    }

    slotColorChanged(currentIndex());
}

void QG_ColorBox::readCustomColorSettings(){
    LC_GROUP_GUARD("ColorBox");
    {
        for (size_t i = 0; i < Max_Custom_Colors; i++) {
            QString colorName = customColorName.arg(i);
            int color = LC_GET_COLOR(colorName, -1);
            if (color < 0) {
                break;
            }
            addColor(static_cast<QRgb>(color), tr("Custom Picked"));
        }
    }
}

void QG_ColorBox::writeCustomColorSettings(){
    LC_GROUP_GUARD("ColorBox");
    {
        int customIndex = 0;
        for (int cIndex = m_colorIndexStart; cIndex < count(); cIndex++) {
            if (itemText(cIndex) == customItemText) {
                auto itemColor = itemData(cIndex).value<QColor>();
                QString colorName = customColorName.arg(customIndex++);
                LC_SET_COLOR(colorName, itemColor.rgb());
            }
        }
    }
}

/**
 * Sets the color shown in the combobox to the given color.
 */
void QG_ColorBox::setColor(const RS_Color& color){
    *m_currentColor = color;
    int lastItem = count()-1;
    if (itemText(lastItem) == userColorText){ // we had user color already in combobox, remove it first
        removeItem(lastItem);
    }

    int cIndex = 0;

    if (color.isByLayer() && m_showByLayer){
        cIndex= 0 + m_showUnchanged; // take care for unchanged, which will be "0" item
    }
    else if (color.isByBlock() && m_showByLayer){
        cIndex= 1 + m_showUnchanged;  // take care for unchanged, which will be "0" item
    }
    else{
        cIndex = findColor(color);
        if ( cIndex == count()) {
            // if we were not able to find any suitable color - we'll add it as the last one in the list
            // this will let us display properly custom colors that were picked by user using
            // color chooser invoked by "Custom" item.
            // otherwise, the custom color will be lost (if we use, say "0" index) and so the
            // combobox will break given color value after editing
            // with this approach, it will return custom color back (if the color will not be changed as part of editing, of course)
            cIndex = addTemporaryCustomColor(color);
        }
    }

    setCurrentIndex(cIndex);

    if (currentIndex() != 0) {
        slotColorChanged(currentIndex());
    }
}

/**
 * generate icon from color, then add to color box
 */
void QG_ColorBox::addColor(QColor color, QString text){
    QPixmap pixmap(":/colors/color00.png");
    int width = pixmap.width();
    int height = pixmap.height();
    QPainter painter(&pixmap);
    painter.fillRect(1, 1, width-2, height-2, color);
    addItem(QIcon(pixmap), text, color);
}

int QG_ColorBox::findColor(const RS_Color& color){
    for (int cIndex = m_colorIndexStart; cIndex < count(); cIndex++) {
        //searching for the color, allowing difference up to 2
        if (getDictionaryDistance(QColor{itemData(cIndex).value<QColor>()}, color) <= 3)
            return cIndex;
    }
    return count();
}

int QG_ColorBox::addCustomColor(const RS_Color& color){
    int current = findColor(color);
    if (current < count()) {
        return current;
    }
    std::vector<int> customIndices;
    for (int cIndex = m_colorIndexStart; cIndex < count(); cIndex++) {
        if (itemText(cIndex) == customItemText) {
            customIndices.push_back(cIndex);
        }
    }
    if (customIndices.size() > Max_Custom_Colors) {
        removeItem(customIndices.front());
    }
    addColor(color.toQColor(), customItemText);

    // save custom colors to settings
    writeCustomColorSettings();

    return count() - 1;
}

/**
 * Sets the color of the pixmap next to the "By Layer" item
 * to the given color.
 */
void QG_ColorBox::setLayerColor(const RS_Color& color) {
    if (!m_showByLayer) {
        return;
    }
    QPixmap pixmap;
    pixmap = QPixmap(":/colors/color00.png");
    int w = pixmap.width();
    int h = pixmap.height();
    QPainter painter(&pixmap);
    painter.fillRect(1, 1, w-2, h-2, color);
    painter.end();

    setItemIcon(0, QIcon(pixmap));
    setItemText(0, tr("By Layer"));

    // needed for the first time a layer is added:
    if (currentIndex()!= count() -1 ) {
        slotColorChanged(currentIndex());
    }
}

/**
 * Called when the color has changed. This method
 * sets the current color to the value chosen or even
 * offers a dialog to the user that allows him/ her to
 * choose an individual color.
 */
void QG_ColorBox::slotColorChanged(int index) {
    m_currentColor->resetFlags();
    if (m_showUnchanged) {
        if (index==0) {
            m_unchanged = true;
        }
        else {
            m_unchanged = false;
        }
    }

    if (m_showByLayer) {
        switch (index-(int)m_showUnchanged) {
        case 0:
            *m_currentColor = RS_Color(RS2::FlagByLayer);
            break;
        case 1:
            *m_currentColor = RS_Color(RS2::FlagByBlock);
            break;
        default:
            break;
        }
    }

    if (itemText(index) == tr("Custom")) {
        RS_Color selectedColor = QColorDialog::getColor(*m_currentColor, this);
        if (selectedColor.isValid()) {
            *m_currentColor = selectedColor;
            int current = addCustomColor(selectedColor);
            if (current < count())
                setCurrentIndex(current);
        }
    }
    else if (index >= m_colorIndexStart) {
        if (index < count()) {
            QVariant q0 = itemData(index);
            if (q0.isValid()) {
                *m_currentColor = itemData(index).value<QColor>();
            }
            else {
                *m_currentColor = Qt::black; //default to black color
            }
        }
    }

    emit colorChanged(*m_currentColor);
}

RS_Color QG_ColorBox::getColor() const{
    return *m_currentColor;
}

bool QG_ColorBox::isUnchanged() const{
    return m_unchanged;
}

int QG_ColorBox::addTemporaryCustomColor(const RS_Color& color)
{   addColor(color.toQColor(), userColorText);
    int index = count() - 1;
    return index;
}
