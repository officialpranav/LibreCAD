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

#include <iostream>
#include "rs_spline.h"
#include "rs_debug.h"
#include "rs_line.h"
#include "rs_painter.h"
#include "rs_pen.h"

RS_SplineData::RS_SplineData(int _degree, bool _closed):
	degree(_degree)
  ,closed(_closed)
{
}

std::ostream& operator << (std::ostream& os, const RS_SplineData& ld) {
    os << "( degree: " << ld.degree <<
       " closed: " << ld.closed;
    if (ld.controlPoints.size()) {
        os << "\n(control points:\n";
        for (auto const& v: ld.controlPoints)
            os<<v;
        os<<")\n";
    }
    if (ld.knotslist.size()) {
        os << "\n(knot vector:\n";
        for (auto const& v: ld.knotslist)
            os<<v;
        os<<")\n";
    }
    os  << ")";
    return os;
}

/**
 * Constructor.
 */
RS_Spline::RS_Spline(RS_EntityContainer* parent,
                     const RS_SplineData& d)
        :RS_EntityContainer(parent), data(d) {
    calculateBorders();
}

RS_Entity* RS_Spline::clone() const{
    auto* l = new RS_Spline(*this);
    l->setOwner(isOwner());
    l->detach();
    return l;
}

void RS_Spline::calculateBorders() {
    /*minV = RS_Vector::minimum(data.startpoint, data.endpoint);
    maxV = RS_Vector::maximum(data.startpoint, data.endpoint);

    QList<RS_Vector>::iterator it;
    for (it = data.controlPoints.begin();
    it!=data.controlPoints.end(); ++it) {

    minV = RS_Vector::minimum(*it, minV);
    maxV = RS_Vector::maximum(*it, maxV);
}
    */
}

void RS_Spline::setDegree(int degree) {
    if (degree>=1 && degree<=3) {
        data.degree = degree;
    } else {
        RS_DEBUG->print(RS_Debug::D_CRITICAL, "%s(%d): invalid degree = %d", __func__, degree, degree);
    }
}

/** @return Degree of this spline curve (1-3).*/
int RS_Spline::getDegree() const{
    return data.degree;
}

size_t RS_Spline::getNumberOfControlPoints() const {
    return data.controlPoints.size();
}

/**
 * @retval true if the spline is closed.
 * @retval false otherwise.
 */
bool RS_Spline::isClosed() const {
		return data.closed;
}

/**
 * Sets the closed flag of this spline.
 */
void RS_Spline::setClosed(bool c) {
    if (data.closed == c)
        return;

    // when switching from closed to open, remove wrapped control points
    if (data.closed && hasWrappedControlPoints()) {
        data.controlPoints.erase(data.controlPoints.begin() + (data.controlPoints.size() - data.degree), data.controlPoints.end());
    }
    data.closed = c;

    update();
}

RS_VectorSolutions RS_Spline::getRefPoints() const{
    return {data.controlPoints};
}

RS_Vector RS_Spline::getNearestRef( const RS_Vector& coord,
                                    double* dist /*= nullptr*/) const{
    // override the RS_EntityContainer method
    // use RS_Entity instead for spline point dragging
    return RS_Entity::getNearestRef(coord, dist);
}

RS_Vector RS_Spline::getNearestSelectedRef( const RS_Vector& coord,
                                            double* dist /*= nullptr*/) const{
    // override the RS_EntityContainer method
    // use RS_Entity instead for spline point dragging
    return RS_Entity::getNearestSelectedRef(coord, dist);
}

/**
 * Updates the internal polygon of this spline. Called when the
 * spline or it's data, position, .. changes.
 */
void RS_Spline::update() {
    RS_DEBUG->print("RS_Spline::update");

    clear();

    if (isUndone()) {
        return;
    }

    if (data.degree<1 || data.degree>3) {
        RS_DEBUG->print("RS_Spline::update: invalid degree: %d", data.degree);
        return;
    }

    // Issue #1689: allow closed splines by 3 control points
    // Issue #1960: DXF import of degree 1 spline with two control points fails in RC 2.2.1_rc4 on windows and MacOS
    if ( (!data.closed && data.controlPoints.size() < size_t(data.degree)+1) || (data.closed && data.controlPoints.size() < 3) ) {

        RS_DEBUG->print("RS_Spline::update: not enough control points");
        return;
    }

    resetBorders();

    int splineSegments = getGraphicVariableInt("$SPLINESEGS", 8);
    std::vector<RS_Vector> p;
    fillStrokePoints(splineSegments, p);

    RS_Vector prev{};
    for (auto const& vp: p) {
        if (prev.valid) {
            auto* line = new RS_Line{this, prev, vp};
            line->setLayer(nullptr);
            line->setPen(RS_Pen(RS2::FlagInvalid));
            addEntity(line);
        }
        prev = vp;
        minV = RS_Vector::minimum(prev, minV);
        maxV = RS_Vector::maximum(prev, maxV);
    }
}

void RS_Spline::fillStrokePoints(int splineSegments, std::vector<RS_Vector>& points) {// wrap control points, if it's not wrapped yet
    std::vector<RS_Vector>& tControlPoints = data.controlPoints;
    if (data.closed && (data.degree == 2 || !hasWrappedControlPoints())) {
        std::vector<RS_Vector> wrappedPoints{data.controlPoints.cbegin(), data.controlPoints.cbegin() + data.degree};
        tControlPoints.insert(tControlPoints.end(), wrappedPoints.cbegin(), wrappedPoints.cend());
        RS_DEBUG->print(RS_Debug::D_NOTICE, "%s: controlPoints: size=%llu\n", __func__, data.controlPoints.size());
    }

    const size_t npts = tControlPoints.size();

    // resolution:
    const size_t  pointsCount = splineSegments * npts;

    // order:
    const size_t  k = data.degree + 1;

    std::vector<double> h(npts+1, 1.);
    points.resize(pointsCount);
    std::fill(points.begin(), points.end(),RS_Vector{0., 0.});
    if (data.closed) {
        rbsplinu(npts, k, pointsCount, tControlPoints, h, points);
    } else {
        rbspline(npts, k, pointsCount, tControlPoints, h, points);
    }
}

RS_Vector RS_Spline::getStartpoint() const {
   if (data.closed) return RS_Vector(false);
   return static_cast<RS_Line*>(const_cast<RS_Spline*>(this)->firstEntity())->getStartpoint();
}

RS_Vector RS_Spline::getEndpoint() const {
   if (data.closed) return RS_Vector(false);
   return static_cast<RS_Line*>(const_cast<RS_Spline*>(this)->lastEntity())->getEndpoint();
}

RS_Vector RS_Spline::getNearestEndpoint(const RS_Vector& coord,
                                        double* dist)const {
    double minDist = RS_MAXDOUBLE;
    RS_Vector ret(false);
    if(! data.closed) { // no endpoint for closed spline
       RS_Vector vp1(getStartpoint());
       RS_Vector vp2(getEndpoint());
       double d1( (coord-vp1).squared());
       double d2( (coord-vp2).squared());
       if( d1<d2){
           ret=vp1;
           minDist=std::sqrt(d1);
       }else{
           ret=vp2;
           minDist=std::sqrt(d2);
       }
//        for (int i=0; i<data.controlPoints.count(); i++) {
//            d = (data.controlPoints.at(i)).distanceTo(coord);

//            if (d<minDist) {
//                minDist = d;
//                ret = data.controlPoints.at(i);
//            }
//        }
    }
	if (dist) {
        *dist = minDist;
    }
    return ret;
}

/*
// The default implementation of RS_EntityContainer is inaccurate but
//   has to do for now..
RS_Vector RS_Spline::getNearestPointOnEntity(const RS_Vector& coord,
        bool onEntity, double* dist, RS_Entity** entity) {
}
*/
RS_Vector RS_Spline::getNearestCenter(const RS_Vector& /*coord*/,
                                      double* dist) const{

    if (dist) {
        *dist = RS_MAXDOUBLE;
    }

    return RS_Vector(false);
}

RS_Vector RS_Spline::getNearestMiddle(const RS_Vector& /*coord*/,
                                      double* dist,
                                      int /*middlePoints*/)const {
    if (dist) {
        *dist = RS_MAXDOUBLE;
    }

    return RS_Vector(false);
}

RS_Vector RS_Spline::getNearestDist(double /*distance*/,
                                    const RS_Vector& /*coord*/,
                                    double* dist) const{
    if (dist)
        *dist = RS_MAXDOUBLE;

    return {};
}

void RS_Spline::move(const RS_Vector& offset) {
    RS_EntityContainer::move(offset);
    for (RS_Vector& vp: data.controlPoints) {
        vp.move(offset);
    }
}

void RS_Spline::rotate(const RS_Vector& center, double angle) {
    rotate(center,RS_Vector(angle));
}

void RS_Spline::rotate(const RS_Vector& center, const RS_Vector& angleVector) {
    RS_EntityContainer::rotate(center, angleVector);
    for (RS_Vector& vp: data.controlPoints) {
        vp.rotate(center, angleVector);
    }
//    update();
}

void RS_Spline::scale(const RS_Vector& center, const RS_Vector& factor) {
    for (RS_Vector& vp: data.controlPoints) {
        vp.scale(center, factor);
    }
    update();
}

RS_Entity& RS_Spline::shear(double k){
    for (RS_Vector& vp: data.controlPoints) {
        vp.shear(k);
    }

    update();
    return *this;
}

void RS_Spline::mirror(const RS_Vector& axisPoint1, const RS_Vector& axisPoint2) {
    for (RS_Vector& vp: data.controlPoints) {
        vp.mirror(axisPoint1, axisPoint2);
    }
    update();
}

void RS_Spline::moveRef(const RS_Vector& ref, const RS_Vector& offset) {
    for (RS_Vector& vp: data.controlPoints) {
        if (ref.distanceTo(vp)<1.0e-4) {
            vp.move(offset);
        }
    }
    update();
}

void RS_Spline::revertDirection() {
    std::reverse(data.controlPoints.begin(), data.controlPoints.end());
}

void RS_Spline::draw(RS_Painter* painter) {
    painter->drawSplineWCS(*this);
}


/**
 * @return The reference points of the spline.
 */
const std::vector<RS_Vector>& RS_Spline::getControlPoints() const{
    return data.controlPoints;
}

/**
 * Appends the given point to the control points.
 */
void RS_Spline::addControlPoint(const RS_Vector& v) {
    if (isClosed() && hasWrappedControlPoints()) {
        data.controlPoints.insert(data.controlPoints.begin() + (data.controlPoints.size() - data.degree), v );
    } else {
        data.controlPoints.push_back(v);
    }
}

/**
 * Removes the control point that was last added.
 */
void RS_Spline::removeLastControlPoint() {
    if (isClosed() && hasWrappedControlPoints()) {
        data.controlPoints.erase(data.controlPoints.begin() + (data.controlPoints.size() - data.degree));
    } else {
        data.controlPoints.pop_back();
    }
}

/**
 * @brief hasWrappedControlPoints whether the control points are wrapped, needed for a closed spline.
 *          only implemented for cubic splines
 * @return bool - true, if the control points are already wrapped.
 *          for a cubic spline with wrapped splines, the last three control points are the same as the first three.
 */
bool RS_Spline::hasWrappedControlPoints() const {
    const std::vector<RS_Vector>& controlPoints = data.controlPoints;
    if (!data.closed || data.degree < 3 || controlPoints.size() < size_t(2 * data.degree) + 1)
        return false;

    return std::equal(controlPoints.cbegin(), controlPoints.cbegin() + data.degree,
               controlPoints.cbegin() + controlPoints.size() - data.degree);
}

//TODO: private interface cleanup; de Boor's Algorithm
/**
 * Generates B-Spline open knot vector with multiplicity
 * equal to the order at the ends.
 */
std::vector<double> RS_Spline::knot(size_t num, size_t order) const{
	if (data.knotslist.size() == num + order) {
		//use custom knot vector
		return data.knotslist;
	}

	std::vector<double> knotVector(num + order, 0.);
	//use uniform knots
	std::iota(knotVector.begin() + order, knotVector.begin() + num + 1, 1);
	std::fill(knotVector.begin() + num + 1, knotVector.end(), knotVector[num]);
	return knotVector;
}

/**
 * Generates rational B-spline basis functions for an open knot vector.
 */
namespace{
    std::vector<double> rbasis(int c, double t, int npts,
                               const std::vector<double>& x,
                               const std::vector<double>& h) {

        int const nplusc = npts + c;

        std::vector<double> temp(nplusc,0.);

        // calculate the first order nonrational basis functions n[i]
        for (int i = 0; i< nplusc-1; i++)
            if ((t >= x[i]) && (t < x[i+1])) temp[i] = 1;

        /* calculate the higher order nonrational basis functions */

        for (int k = 2; k <= c; k++) {
            for (int i = 0; i < nplusc-k; i++) {
// if the lower order basis function is zero skip the calculation
                if (temp[i] != 0)
                    temp[i] = ((t-x[i])*temp[i])/(x[i+k-1]-x[i]);
                // if the lower order basis function is zero skip the calculation
                if (temp[i+1] != 0)
                    temp[i] += ((x[i+k]-t)*temp[i+1])/(x[i+k]-x[i+1]);
            }
        }

        // pick up last point
        if (t >= x[nplusc-1]) temp[npts-1] = 1;

        // calculate sum for denominator of rational basis functions
        double sum = 0.;
        for (int i = 0; i < npts; i++) {
            sum += temp[i]*h[i];
        }

        std::vector<double> r(npts, 0);
        // form rational basis functions and put in r vector
        if (sum != 0) {
            for (int i = 0; i < npts; i++)
                r[i] = (temp[i]*h[i])/sum;
        }
        return r;
    }
}


/**
 * Generates a rational B-spline curve using a uniform open knot vector.
 */
void RS_Spline::rbspline(size_t npts, size_t k, size_t p1,
                         const std::vector<RS_Vector>& b,
                         const std::vector<double>& h,
                         std::vector<RS_Vector>& p) const{
	size_t const nplusc = npts + k;

	// generate the open knot vector
	auto const x = knot(npts, k);

    // calculate the points on the rational B-spline curve
    double t {x[0]};
    double const step {(x[nplusc-1] - t) / (p1-1)};

	for (auto& vp: p) {
		if (x[nplusc-1] - t < 5e-6) t = x[nplusc-1];

        // generate the basis function for this value of t
		auto const nbasis = rbasis(k, t, npts, x, h);

        // generate a point on the curve
		for (size_t i = 0; i < npts; i++)
			vp += b[i] * nbasis[i];

		t += step;
    }

}

std::vector<double> RS_Spline::knotu(size_t num, size_t order) const{
    if (data.knotslist.size() == num + order) {
//use custom knot vector
        return data.knotslist;
    }
    std::vector<double> knotVector(num + order, 0.);
    std::iota(knotVector.begin(), knotVector.end(), 0);
    return knotVector;
}

void RS_Spline::rbsplinu(size_t npts, size_t k, size_t p1,
                         const std::vector<RS_Vector>& b,
                         const std::vector<double>& h,
                         std::vector<RS_Vector>& p) const{
    size_t const nplusc = npts + k;

/* generate the periodic knot vector */
    std::vector<double> const x = knotu(npts, k);

    /*    calculate the points on the rational B-spline curve */
    double t = k-1;
    double const step = double(npts - k + 1)/(p1 - 1);

    for (auto& vp: p) {
        if (x[nplusc-1] - t < 5e-6) t = x[nplusc-1];

/* generate the basis function for this value of t */
        auto const nbasis = rbasis(k, t, npts, x, h);
/* generate a point on the curve, for x, y, z */
        for (size_t i = 0; i < npts; i++)
            vp += b[i] * nbasis[i];

        t += step;
    }
}

/**
 * Dumps the spline's data to stdout.
 */
std::ostream& operator << (std::ostream& os, const RS_Spline& l) {
    os << " Spline: " << l.getData() << "\n";
    return os;
}

RS_Vector RS_Spline::getNearestPointOnEntity(const RS_Vector &coord, bool onEntity, double *dist, RS_Entity **entity) const {
    return RS_EntityContainer::getNearestPointOnEntity(coord, onEntity, dist, entity);
/*    RS_Vector point(false);

    RS_Entity *en = getNearestEntity(coord, dist, RS2::ResolveNone);

    if (en && en->isVisible()
        && !en->getParent()->ignoredSnap()
        ) {
        point = en->getNearestPointOnEntity(coord, onEntity, dist, entity);
    }

    return point;*/
}
