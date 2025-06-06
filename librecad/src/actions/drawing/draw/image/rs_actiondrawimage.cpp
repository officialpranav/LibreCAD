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

#include "rs_actiondrawimage.h"

#include <QImage>

#include "lc_graphicviewport.h"
#include "qg_imageoptions.h"
#include "rs_creation.h"
#include "rs_dialogfactory.h"
#include "rs_dialogfactoryinterface.h"
#include "rs_document.h"
#include "rs_image.h"
#include "rs_units.h"

struct RS_ActionDrawImage::ImageData {
	RS_ImageData data;
	QImage img;
};

/**
 * Constructor.
 */
RS_ActionDrawImage::RS_ActionDrawImage(LC_ActionContext *actionContext)
    :RS_PreviewActionInterface("Image", actionContext, RS2::ActionDrawImage)
    , m_imageData(std::make_unique<ImageData>())
	, m_lastStatus(ShowDialog){
}

RS_ActionDrawImage::~RS_ActionDrawImage() = default;


void RS_ActionDrawImage::init(int status){
    RS_PreviewActionInterface::init(status);

    reset();

    m_imageData->data.file = RS_DIALOGFACTORY->requestImageOpenDialog();
    // RVT_PORT should we really redarw here?? graphicView->redraw();

    if (!m_imageData->data.file.isEmpty()){
//std::cout << "file: " << pImg->data.file << "\n";
//qDebug() << "file: " << pImg->data.file;

        m_imageData->img = QImage(m_imageData->data.file);

        setStatus(SetTargetPoint);
    } else {
        setFinished();
        //RS_DIALOGFACTORY->requestToolBar(RS2::ToolBarMain);
    }
}

void RS_ActionDrawImage::reset() {
	m_imageData->data = {
				   0,
				   {0.0,0.0},
				   {1.0,0.0},
				   {0.0,1.0},
				   {1.0,1.0},
				   "",
				   50, 50, 0
			   };
}

void RS_ActionDrawImage::doTrigger() {
    if (!m_imageData->data.file.isEmpty()){
        RS_Creation creation(m_container, m_viewport);
        creation.createImage(&m_imageData->data);
    }

    m_viewport->zoomAuto();
    finish(false);
}

void RS_ActionDrawImage::onMouseMoveEvent(int status, LC_MouseEvent *e) {
    if (status == SetTargetPoint){
        bool snappedToRelZero = trySnapToRelZeroCoordinateEvent(e);
        if (!snappedToRelZero){
            m_imageData->data.insertionPoint = e->snapPoint;
            // fixme - ucs - review this code
//RS_Creation creation(preview, nullptr, false);
            //creation.createInsert(data);
            double const w = m_imageData->img.width();
            double const h = m_imageData->img.height();
            previewLine({0., 0.}, {w, 0.});
            previewLine({w, 0.}, {w, h});
            previewLine({w, h}, {0., h});
            previewLine({0., h}, {0., 0.});

            m_preview->scale({0., 0.},
                           {m_imageData->data.uVector.magnitude(), m_imageData->data.uVector.magnitude()});
            m_preview->rotate({0., 0.}, m_imageData->data.uVector.angle());
            m_preview->move(m_imageData->data.insertionPoint);
        }
    }
}

void RS_ActionDrawImage::onMouseLeftButtonRelease([[maybe_unused]]int status, LC_MouseEvent *e) {
    fireCoordinateEventForSnap(e);
}

void RS_ActionDrawImage::onMouseRightButtonRelease([[maybe_unused]]int status, [[maybe_unused]]LC_MouseEvent *e) {
    finish(false);
}

void RS_ActionDrawImage::onCoordinateEvent([[maybe_unused]]int status, [[maybe_unused]]bool isZero, const RS_Vector &pos) {
    m_imageData->data.insertionPoint = pos;
    trigger();
}

bool RS_ActionDrawImage::doProcessCommand(int status, const QString &c) {
    bool accept = false;
    switch (status) {
        case SetTargetPoint: {
            if (checkCommand("angle", c)){
                deletePreview();
                m_lastStatus = (Status) status;
                setStatus(SetAngle);
                accept = true;
            } else if (checkCommand("factor", c)){
                deletePreview();
                m_lastStatus = (Status) status;
                setStatus(SetFactor);
                accept = true;
            } else if (checkCommand("dpi", c)){
                deletePreview();
                m_lastStatus = (Status) status;
                setStatus(SetDPI);
                accept = true;
            }
            break;
        }
        case SetAngle: {
            double wcsAngle;
            bool ok = parseToWCSAngle(c, wcsAngle);
            if (ok){
                setAngle(wcsAngle);
                accept = true;
            } else {
                commandMessage(tr("Not a valid expression"));
            }
            updateOptions();
            setStatus(m_lastStatus);
            break;
        }
        case SetFactor: {
            bool ok;
            double f = RS_Math::eval(c, &ok);
            if (ok){
                setFactor(f);
                accept = true;
            } else {
                commandMessage(tr("Not a valid expression"));
            }
            updateOptions();
            setStatus(m_lastStatus);
            break;
        }
        case SetDPI : {
            bool ok;
            double dpi = RS_Math::eval(c, &ok);

            if (ok){
                setFactor(RS_Units::dpiToScale(dpi, m_document->getGraphicUnit()));
                accept = true;
            } else {
                commandMessage(tr("Not a valid expression"));
            }
            updateOptions();
            setStatus(m_lastStatus);
            break;
        }
        default:
            break;
    }
    return accept;
}

double RS_ActionDrawImage::getUcsAngleDegrees() const{
    return toUCSBasisAngleDegrees(m_imageData->data.uVector.angle());
}

void RS_ActionDrawImage::setUcsAngleDegrees(double ucsRelAngleDegrees){
    double wcsAngle = toWorldAngleFromUCSBasisDegrees(ucsRelAngleDegrees);
    setAngle(wcsAngle);
}

void RS_ActionDrawImage::setAngle(double wcsAngle) const{
    double l = m_imageData->data.uVector.magnitude();
    m_imageData->data.uVector.setPolar(l, wcsAngle);
    m_imageData->data.vVector.setPolar(l, wcsAngle + M_PI_2);
}

double RS_ActionDrawImage::getFactor() const{
    return m_imageData->data.uVector.magnitude();
}

void RS_ActionDrawImage::setFactor(double f) const{
    double a = m_imageData->data.uVector.angle();
    m_imageData->data.uVector.setPolar(f, a);
    m_imageData->data.vVector.setPolar(f, a + M_PI_2);
}

double RS_ActionDrawImage::dpiToScale(double dpi) const{
    return RS_Units::dpiToScale(dpi, m_document->getGraphicUnit());
}

double RS_ActionDrawImage::scaleToDpi(double scale) const{
    return RS_Units::scaleToDpi(scale, m_document->getGraphicUnit());
}

RS2::CursorType RS_ActionDrawImage::doGetMouseCursor([[maybe_unused]] int status){
    return RS2::CadCursor;
}

QStringList RS_ActionDrawImage::getAvailableCommands(){
    QStringList cmd;

    switch (getStatus()) {
        case SetTargetPoint:
            cmd += command("angle");
            cmd += command("factor");
            cmd += command("dpi");
            break;
        default:
            break;
    }
    return cmd;
}

void RS_ActionDrawImage::updateMouseButtonHints(){
    switch (getStatus()) {
        case SetTargetPoint:
            updateMouseWidgetTRCancel(tr("Specify reference point"), MOD_SHIFT_RELATIVE_ZERO);
            break;
        case SetAngle:
            updateMouseWidget(tr("Enter angle:"));
            break;
        case SetFactor:
            updateMouseWidget(tr("Enter factor:"));
            break;
        case SetDPI:
            updateMouseWidget(tr("Enter dpi:"));
            break;
        default:
            updateMouseWidget();
            break;
    }
}

LC_ActionOptionsWidget* RS_ActionDrawImage::createOptionsWidget() {
    return new QG_ImageOptions();
}
