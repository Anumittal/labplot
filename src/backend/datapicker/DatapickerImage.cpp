/***************************************************************************
    File                 : DatapickerImage.cpp
    Project              : LabPlot
    Description          : Worksheet for Datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
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

#include "DatapickerImage.h"
#include "DatapickerImagePrivate.h"
#include "backend/datapicker/ImageEditor.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "backend/worksheet/Worksheet.h"
#include "commonfrontend/datapicker/DatapickerImageView.h"
#include "backend/datapicker/Segments.h"

#include <QDesktopWidget>
#include <QGraphicsScene>
#include <QMenu>

#include <KIcon>
#include <KConfigGroup>
#include <KLocale>

/**
 * \class DatapickerImage
 * \brief container to open image/plot.
 *
 * Top-level container for DatapickerPoint.
 *
 * * \ingroup datapicker
 */
DatapickerImage::DatapickerImage(AbstractScriptingEngine* engine, const QString& name, bool loading)
	: AbstractPart(name), scripted(engine),
	  isLoaded(false),
	  m_magnificationWindow(0),
	  d(new DatapickerImagePrivate(this)),
	  m_segments(new Segments(this)),
	  m_editor(new ImageEditor()) {

	if (!loading)
		init();
}

DatapickerImage::~DatapickerImage() {
	delete m_segments;
	delete m_editor;
	delete d;
}

void DatapickerImage::init() {
	KConfig config;
	KConfigGroup group = config.group( "DatapickerImage" );
	d->fileName = group.readEntry("FileName", QString());
	d->rotationAngle = group.readEntry("RotationAngle", 0.0);
	d->minSegmentLength = group.readEntry("MinSegmentLength", 30);
	d->pointSeparation = group.readEntry("PointSeparation", 30);
	d->axisPoints.type = (DatapickerImage::GraphType) group.readEntry("GraphType", (int) DatapickerImage::Cartesian);
	d->axisPoints.ternaryScale = group.readEntry("TernaryScale", 1);
	d->settings.type = (DatapickerImage::ColorAttributes) group.readEntry("ColorAttributesType", (int) DatapickerImage::Intensity);
	d->settings.foregroundThresholdHigh = group.readEntry("ForegroundThresholdHigh", 10);
	d->settings.foregroundThresholdLow = group.readEntry("ForegroundThresholdLow", 0);
	d->settings.hueThresholdHigh = group.readEntry("HueThresholdHigh", 360);
	d->settings.hueThresholdLow = group.readEntry("HueThresholdLow", 180);
	d->settings.intensityThresholdHigh = group.readEntry("IntensityThresholdHigh", 50);
	d->settings.intensityThresholdLow = group.readEntry("IntensityThresholdLow", 0);
	d->settings.saturationThresholdHigh = group.readEntry("SaturationThresholdHigh", 100);
	d->settings.saturationThresholdLow = group.readEntry("SaturationThresholdLow", 50);
	d->settings.valueThresholdHigh = group.readEntry("ValueThresholdHigh", 50);
	d->settings.valueThresholdLow = group.readEntry("ValueThresholdLow", 0);
	d->plotPointsType = (DatapickerImage::PointsType) group.readEntry("PlotPointsType", (int) DatapickerImage::AxisPoints);

	d->plotImageType = DatapickerImage::OriginalImage;
}


/*!
    Returns an icon to be used in the project explorer.
*/
QIcon DatapickerImage::icon() const {
	return KIcon("image-x-generic");
}

/*!
    Return a new context menu
*/
QMenu* DatapickerImage::createContextMenu() {
	QMenu* menu = new QMenu(0);
	emit requestProjectContextMenu(menu);
	return menu;
}

void DatapickerImage::createContextMenu(QMenu* menu) {
	emit requestProjectContextMenu(menu);
}

//! Construct a primary view on me.
/**
 * This method may be called multiple times during the life time of an Aspect, or it might not get
 * called at all. Aspects must not depend on the existence of a view for their operation.
 */
QWidget* DatapickerImage::view() const {
	if (!m_view) {
		m_view = new DatapickerImageView(const_cast<DatapickerImage *>(this));
		connect(m_view, SIGNAL(statusInfo(QString)), this, SIGNAL(statusInfo(QString)));
	}
	return m_view;
}

/*!
    Selects or deselects the Datapicker/DatapickerImage in the project explorer.
    This function is called in \c DatapickerImageView.
    The DatapickerImage gets deselected if there are selected items in the view,
    and selected if there are no selected items in the view.
*/
void DatapickerImage::setSelectedInView(const bool b) {
	if (b)
		emit childAspectSelectedInView(this);
	else
		emit childAspectDeselectedInView(this);
}

QGraphicsScene* DatapickerImage::scene() const {
	return d->m_scene;
}

QRectF DatapickerImage::pageRect() const {
	return d->m_scene->sceneRect();
}

void DatapickerImage::setPlotImageType(const DatapickerImage::PlotImageType type) {
	d->plotImageType = type;
	emit requestUpdate();
}

DatapickerImage::PlotImageType DatapickerImage::plotImageType() {
	return d->plotImageType;
}

void DatapickerImage::setSegmentVisible(bool on) {
	m_segments->setSegmentsVisible(on);
}

void DatapickerImage::initSceneParameters() {
	setRotationAngle(0.0);
	setminSegmentLength(30);
	setPointSeparation(30);

	ReferencePoints axisPoints;
	axisPoints.ternaryScale = 1;
	axisPoints.type = DatapickerImage::Cartesian;
	setAxisPoints(axisPoints);

	EditorSettings settings;
	settings.type = DatapickerImage::Intensity;
	settings.foregroundThresholdHigh = 10;
	settings.foregroundThresholdLow = 0;
	settings.hueThresholdHigh = 360;
	settings.hueThresholdLow = 180;
	settings.intensityThresholdHigh = 50;
	settings.intensityThresholdLow = 0;
	settings.saturationThresholdHigh = 100;
	settings.saturationThresholdLow = 50;
	settings.valueThresholdHigh = 50;
	settings.valueThresholdLow = 0;
	setSettings(settings);

	PointsType plotPointsType = DatapickerImage::AxisPoints;
	setPlotPointsType(plotPointsType);
}

/* =============================== getter methods for background options ================================= */
CLASS_D_READER_IMPL(DatapickerImage, QString, fileName, fileName)
CLASS_D_READER_IMPL(DatapickerImage, DatapickerImage::ReferencePoints, axisPoints, axisPoints)
CLASS_D_READER_IMPL(DatapickerImage, DatapickerImage::EditorSettings, settings, settings)
BASIC_D_READER_IMPL(DatapickerImage, float, rotationAngle, rotationAngle)
BASIC_D_READER_IMPL(DatapickerImage, DatapickerImage::PointsType, plotPointsType, plotPointsType)
BASIC_D_READER_IMPL(DatapickerImage, int, pointSeparation, pointSeparation)
BASIC_D_READER_IMPL(DatapickerImage, int, minSegmentLength, minSegmentLength)
/* ============================ setter methods and undo commands  for background options  ================= */
STD_SETTER_CMD_IMPL_F_S(DatapickerImage, SetFileName, QString, fileName, updateFileName)
void DatapickerImage::setFileName(const QString& fileName) {
	if (fileName!= d->fileName) {
		beginMacro(i18n("%1: upload new image", name()));
		exec(new DatapickerImageSetFileNameCmd(d, fileName, i18n("%1: upload image")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_S(DatapickerImage, SetRotationAngle, float, rotationAngle)
void DatapickerImage::setRotationAngle(float angle) {
	if (angle != d->rotationAngle)
		exec(new DatapickerImageSetRotationAngleCmd(d, angle, i18n("%1: set rotation angle")));
}

STD_SETTER_CMD_IMPL_S(DatapickerImage, SetAxisPoints, DatapickerImage::ReferencePoints, axisPoints)
void DatapickerImage::setAxisPoints(const DatapickerImage::ReferencePoints& points) {
	if (memcmp(&points, &d->axisPoints, sizeof(points)) != 0)
		exec(new DatapickerImageSetAxisPointsCmd(d, points, i18n("%1: set Axis points")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerImage, SetSettings, DatapickerImage::EditorSettings, settings, discretize)
void DatapickerImage::setSettings(const DatapickerImage::EditorSettings& editorSettings) {
	if (memcmp(&editorSettings, &d->settings, sizeof(editorSettings)) != 0)
		exec(new DatapickerImageSetSettingsCmd(d, editorSettings, i18n("%1: set editor settings")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerImage, SetMinSegmentLength, int, minSegmentLength, makeSegments)
void DatapickerImage::setminSegmentLength(const int value) {
	if (d->minSegmentLength != value)
		exec(new DatapickerImageSetMinSegmentLengthCmd(d, value, i18n("%1: set minimum segment length")));        ;
}

void DatapickerImage::setPrinting(bool on) const {
	QList<WorksheetElement*> childElements = parentAspect()->children<WorksheetElement>(AbstractAspect::Recursive | AbstractAspect::IncludeHidden);
	foreach(WorksheetElement* elem, childElements)
		elem->setPrinting(on);
}

void DatapickerImage::setPlotPointsType(const PointsType pointsType) {
	d->plotPointsType = pointsType;
}

void DatapickerImage::setPointSeparation(const int value) {
	d->pointSeparation = value;
}

//##############################################################################
//######################  Private implementation ###############################
//##############################################################################
DatapickerImagePrivate::DatapickerImagePrivate(DatapickerImage *owner):q(owner),
	pageRect(0, 0, 1500, 1500),
	m_scene(new QGraphicsScene(pageRect)) {
}

QString DatapickerImagePrivate::name() const {
	return q->name();
}

bool DatapickerImagePrivate::uploadImage(const QString& address) {
	bool rc = q->originalPlotImage.load(address);
	if (rc) {
		q->processedPlotImage = q->originalPlotImage;
		if (plotImageType == DatapickerImage::ProcessedImage)
			discretize();

		//resize the screen
		double w = Worksheet::convertToSceneUnits(q->originalPlotImage.width(), Worksheet::Inch)/QApplication::desktop()->physicalDpiX();
		double h = Worksheet::convertToSceneUnits(q->originalPlotImage.height(), Worksheet::Inch)/QApplication::desktop()->physicalDpiX();
		m_scene->setSceneRect(0, 0, w, h);
		q->isLoaded = true;
		emit q->requestUpdate();
	}
	return rc;
}

void DatapickerImagePrivate::discretize() {
	q->m_editor->discretize(&q->processedPlotImage, &q->originalPlotImage, settings);

	//update segments
	makeSegments();
}

void DatapickerImagePrivate::makeSegments() {
	q->m_segments->makeSegments(q->processedPlotImage);
	if (plotPointsType == DatapickerImage::SegmentPoints)
		q->m_segments->setSegmentsVisible(true);

	emit q->requestUpdate();
}

DatapickerImagePrivate::~DatapickerImagePrivate() {
	delete m_scene;
}

void DatapickerImagePrivate::updateFileName() {
	WAIT_CURSOR;
	QList<WorksheetElement*> childElements = q->parentAspect()->children<WorksheetElement>(AbstractAspect::Recursive | AbstractAspect::IncludeHidden);
	if (childElements.count()) {
		foreach(WorksheetElement* elem, childElements)
			elem->remove();
	}

	q->isLoaded = false;
	const QString& address = fileName.trimmed();

	if ( !address.isEmpty() ) {
		if (uploadImage(address)) {
			q->initSceneParameters();
			fileName = address;
		}
	} else {
		//hide segments if they are visible
		q->m_segments->setSegmentsVisible(false);
	}

	RESET_CURSOR;
}
//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

//! Save as XML
void DatapickerImage::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement( "datapickerImage" );
	writeBasicAttributes(writer);
	writeCommentElement(writer);
	//general properties
	writer->writeStartElement( "general" );
	writer->writeAttribute( "fileName", d->fileName );
	writer->writeAttribute( "plotPointsType", QString::number(d->plotPointsType) );
	writer->writeEndElement();

	writer->writeStartElement( "axisPoint" );
	writer->writeAttribute( "graphType", QString::number(d->axisPoints.type) );
	writer->writeAttribute( "ternaryScale", QString::number(d->axisPoints.ternaryScale) );
	writer->writeAttribute( "axisPointLogicalX1", QString::number(d->axisPoints.logicalPos[0].x()) );
	writer->writeAttribute( "axisPointLogicalY1", QString::number(d->axisPoints.logicalPos[0].y()) );
	writer->writeAttribute( "axisPointLogicalX2", QString::number(d->axisPoints.logicalPos[1].x()) );
	writer->writeAttribute( "axisPointLogicalY2", QString::number(d->axisPoints.logicalPos[1].y()) );
	writer->writeAttribute( "axisPointLogicalX3", QString::number(d->axisPoints.logicalPos[2].x()) );
	writer->writeAttribute( "axisPointLogicalY3", QString::number(d->axisPoints.logicalPos[2].y()) );
	writer->writeAttribute( "axisPointLogicalZ1", QString::number(d->axisPoints.logicalPos[0].z()) );
	writer->writeAttribute( "axisPointLogicalZ2", QString::number(d->axisPoints.logicalPos[1].z()) );
	writer->writeAttribute( "axisPointLogicalZ3", QString::number(d->axisPoints.logicalPos[2].z()) );
	writer->writeAttribute( "axisPointSceneX1", QString::number(d->axisPoints.scenePos[0].x()) );
	writer->writeAttribute( "axisPointSceneY1", QString::number(d->axisPoints.scenePos[0].y()) );
	writer->writeAttribute( "axisPointSceneX2", QString::number(d->axisPoints.scenePos[1].x()) );
	writer->writeAttribute( "axisPointSceneY2", QString::number(d->axisPoints.scenePos[1].y()) );
	writer->writeAttribute( "axisPointSceneX3", QString::number(d->axisPoints.scenePos[2].x()) );
	writer->writeAttribute( "axisPointSceneY3", QString::number(d->axisPoints.scenePos[2].y()) );
	writer->writeEndElement();

	//editor and segment settings
	writer->writeStartElement( "editorSettings" );
	writer->writeAttribute( "plotImageType", QString::number(d->plotImageType) );
	writer->writeAttribute( "rotationAngle", QString::number(d->rotationAngle) );
	writer->writeAttribute( "minSegmentLength", QString::number(d->minSegmentLength) );
	writer->writeAttribute( "pointSeparation", QString::number(d->pointSeparation) );
	writer->writeAttribute( "colorAttributesType", QString::number(d->settings.type) );
	writer->writeAttribute( "foregroundThresholdHigh", QString::number(d->settings.foregroundThresholdHigh) );
	writer->writeAttribute( "foregroundThresholdLow", QString::number(d->settings.foregroundThresholdLow) );
	writer->writeAttribute( "hueThresholdHigh", QString::number(d->settings.hueThresholdHigh) );
	writer->writeAttribute( "hueThresholdLow", QString::number(d->settings.hueThresholdLow) );
	writer->writeAttribute( "intensityThresholdHigh", QString::number(d->settings.intensityThresholdHigh) );
	writer->writeAttribute( "intensityThresholdLow", QString::number(d->settings.intensityThresholdLow) );
	writer->writeAttribute( "saturationThresholdHigh", QString::number(d->settings.saturationThresholdHigh) );
	writer->writeAttribute( "saturationThresholdLow", QString::number(d->settings.saturationThresholdLow) );
	writer->writeAttribute( "valueThresholdHigh", QString::number(d->settings.valueThresholdHigh) );
	writer->writeAttribute( "valueThresholdLow", QString::number(d->settings.valueThresholdLow) );
	writer->writeEndElement();

	//serialize all children
	QList<AbstractAspect *> childrenAspect = children<AbstractAspect>(IncludeHidden);
	foreach(AbstractAspect *child, childrenAspect)
		child->save(writer);

	writer->writeEndElement();
}

//! Load from XML
bool DatapickerImage::load(XmlStreamReader* reader) {
	if(!reader->isStartElement() || reader->name() != "datapickerImage") {
		reader->raiseError(i18n("no image element found"));
		return false;
	}

	if (!readBasicAttributes(reader))
		return false;

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "datapickerImage")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if (reader->name() == "general") {
			attribs = reader->attributes();

			str = attribs.value("fileName").toString();
			d->fileName = str;

			str = attribs.value("plotPointsType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("plotPointsType"));
			else
				d->plotPointsType = DatapickerImage::PointsType(str.toInt());

		} else if (reader->name() == "axisPoint") {
			attribs = reader->attributes();

			str = attribs.value("graphType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("graphType"));
			else
				d->axisPoints.type = DatapickerImage::GraphType(str.toInt());

			str = attribs.value("ternaryScale").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("ternaryScale"));
			else
				d->axisPoints.ternaryScale = str.toDouble();

			str = attribs.value("axisPointLogicalX1").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("axisPointLogicalX1"));
			else
				d->axisPoints.logicalPos[0].setX(str.toDouble());

			str = attribs.value("axisPointLogicalY1").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("axisPointLogicalY1"));
			else
				d->axisPoints.logicalPos[0].setY(str.toDouble());

			str = attribs.value("axisPointLogicalZ1").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("axisPointLogicalZ1"));
			else
				d->axisPoints.logicalPos[0].setZ(str.toDouble());

			str = attribs.value("axisPointLogicalX2").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("axisPointLogicalX2"));
			else
				d->axisPoints.logicalPos[1].setX(str.toDouble());

			str = attribs.value("axisPointLogicalY2").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("axisPointLogicalY2"));
			else
				d->axisPoints.logicalPos[1].setY(str.toDouble());

			str = attribs.value("axisPointLogicalZ2").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("axisPointLogicalZ2"));
			else
				d->axisPoints.logicalPos[1].setZ(str.toDouble());

			str = attribs.value("axisPointLogicalX3").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("axisPointLogicalX3"));
			else
				d->axisPoints.logicalPos[2].setX(str.toDouble());

			str = attribs.value("axisPointLogicalY3").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("axisPointLogicalY3"));
			else
				d->axisPoints.logicalPos[2].setY(str.toDouble());

			str = attribs.value("axisPointLogicalZ3").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("axisPointLogicalZ3"));
			else
				d->axisPoints.logicalPos[2].setZ(str.toDouble());

			str = attribs.value("axisPointSceneX1").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("axisPointSceneX1"));
			else
				d->axisPoints.scenePos[0].setX(str.toDouble());

			str = attribs.value("axisPointSceneY1").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("axisPointSceneY1"));
			else
				d->axisPoints.scenePos[0].setY(str.toDouble());

			str = attribs.value("axisPointSceneX2").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("axisPointSceneX2"));
			else
				d->axisPoints.scenePos[1].setX(str.toDouble());

			str = attribs.value("axisPointSceneY2").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("axisPointSceneY2"));
			else
				d->axisPoints.scenePos[1].setY(str.toDouble());

			str = attribs.value("axisPointSceneX3").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("axisPointSceneX3"));
			else
				d->axisPoints.scenePos[2].setX(str.toDouble());

			str = attribs.value("axisPointSceneY3").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("axisPointSceneY3"));
			else
				d->axisPoints.scenePos[2].setY(str.toDouble());

		} else if (reader->name() == "editorSettings") {
			attribs = reader->attributes();

			str = attribs.value("plotImageType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("plotImageType"));
			else
				d->plotImageType = DatapickerImage::PlotImageType(str.toInt());

			str = attribs.value("rotationAngle").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("rotationAngle"));
			else
				d->rotationAngle = str.toFloat();

			str = attribs.value("minSegmentLength").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("minSegmentLength"));
			else
				d->minSegmentLength = str.toInt();

			str = attribs.value("pointSeparation").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("pointSeparation"));
			else
				d->pointSeparation = str.toInt();

			str = attribs.value("colorAttributesType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("colorAttributesType"));
			else
				d->settings.type = DatapickerImage::ColorAttributes(str.toInt());

			str = attribs.value("foregroundThresholdHigh").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("foregroundThresholdHigh"));
			else
				d->settings.foregroundThresholdHigh = str.toInt();

			str = attribs.value("foregroundThresholdLow").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("foregroundThresholdLow"));
			else
				d->settings.foregroundThresholdLow = str.toInt();

			str = attribs.value("hueThresholdHigh").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("hueThresholdHigh"));
			else
				d->settings.hueThresholdHigh = str.toInt();

			str = attribs.value("hueThresholdLow").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("hueThresholdLow"));
			else
				d->settings.hueThresholdLow = str.toInt();

			str = attribs.value("intensityThresholdHigh").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("intensityThresholdHigh"));
			else
				d->settings.intensityThresholdHigh = str.toInt();

			str = attribs.value("intensityThresholdLow").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("intensityThresholdLow"));
			else
				d->settings.intensityThresholdLow = str.toInt();

			str = attribs.value("saturationThresholdHigh").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("saturationThresholdHigh"));
			else
				d->settings.saturationThresholdHigh = str.toInt();

			str = attribs.value("saturationThresholdLow").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("saturationThresholdLow"));
			else
				d->settings.saturationThresholdLow = str.toInt();

			str = attribs.value("valueThresholdHigh").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("valueThresholdHigh"));
			else
				d->settings.valueThresholdHigh = str.toInt();

			str = attribs.value("valueThresholdLow").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("valueThresholdLow"));
			else
				d->settings.valueThresholdLow = str.toInt();

		} else if(reader->name() == "datapickerPoint") {
			DatapickerPoint* datapickerPoint = new DatapickerPoint("");
			datapickerPoint->setHidden(true);
			if (!datapickerPoint->load(reader)) {
				delete datapickerPoint;
				return false;
			} else {
				addChild(datapickerPoint);
			}
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	d->uploadImage(d->fileName);
	return true;
}
