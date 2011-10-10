/***************************************************************************
    File                 : Set.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : generic set class
                           
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#ifndef SET_H
#define SET_H

#include "Label.h"
#include "Point.h"
#include "Range.h"
#include "Style.h"
#include <QString>

class Set{

public:
	// type of set (GRASS,VTK obsolete)
	enum SetType {SET2D,SET3D,SETM,SETGRASS,SETVTK,SET4D,SETIMAGE,SETL};

	Set();
	Set(Set::SetType);
// 	Set(QString name="", int number=100); //TODO remove

	void setType(const Set::SetType);
	Set::SetType type() const;

	ACCESS(QString, functionName, FunctionName);
	ACCESSFLAG(m_shown, Shown);

	Style* style(){ return &m_style; }
	void setStyle(Style s) { m_style = s; }

	Label* label() { return &m_label; }
	void setLabel(Label label) { m_label = label; }

	//TODO Functions for "Tick labels".

	void drawStyle(QPainter *p, int x, int y);	//!< draw style and symbol on QPainter

	QList<Point> list_data;
	QList<Range> list_ranges;
	QList<int> list_numbers;


	/*	Set(QString n="", QString label="", LSource src=SFUNCTION, PType t=P2D, Style *st=0,
			Symbol *sy=0,int nr=0, bool s=true);
	virtual ~Set() {}
	void openSet(QTextStream *t,int version);
	void saveSet(QTextStream *t);
	virtual void saveXML(QDomDocument doc, QDomElement graphtag) = 0;
	QDomElement saveGraphXML(QDomDocument doc, int gtype);
	virtual void openXML(QDomNode node) = 0;
	void openGraphXML(QDomElement e);
	QString Name() { return name; }
*/

/*	LSource Source() { return source; }
	void setSource(LSource s) { source=s; }
	int ReadAs() { return readas; }
	void setReadAs(int r) { readas = r; }
	PType Type() { return type; }
*/

/*	AnnotateValues getAnnotateValues() { return av; }
	void setAnnotateValues(AnnotateValues a) { av=a;}
	QString FitFunction() { return fitfunction; }
	void setFitFunction(QString f) { fitfunction=f; }
*/

protected:
	SetType m_type;
	QString m_functionName;
	Style m_style;
	Label m_label;

	//TODO???
// 	int m_number;
	bool m_shown;		//!< shown/hidden

/*	PType type;		//!< plot type of a graph
	LSource source;
	int readas;		//!< selected read as for data
*/

//	AnnotateValues av;
//	QString fitfunction;
};

#endif // Set_H