/***************************************************************************
    File                 : TemplateHandler.h
    Project              : LabPlot/SciDAVis
    Description          : Widget for handling saving and loading of templates
    --------------------------------------------------------------------
	Copyright            : (C) 2012 by Stefan Gerlach (stefan.gerlach*uni-konstanz.de)
															Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses)
                           
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

#ifndef TEMPLATEHANDLER_H
#define TEMPLATEHANDLER_H

#include <QtGui/QHBoxLayout>
#include <QtGui/QSpacerItem>
#include <QtGui/QToolButton>

#include <KConfig>

class TemplateHandler : public QWidget{
	Q_OBJECT

	public:
		enum ClassName {Spreadsheet, Worksheet, CartesianPlot, XYCurve, Axis};
		
		TemplateHandler(QWidget* parent, ClassName);
		~TemplateHandler();
		
	private:
		void retranslateUi();

		ClassName className;
		QList<QString> dirNames;

		QHBoxLayout *horizontalLayout;
		QSpacerItem *horizontalSpacer;
		QToolButton *tbLoad;
		QToolButton *tbSave;
		QToolButton *tbSaveDefault;
		QSpacerItem *horizontalSpacer2;
		QToolButton *tbCopy;
		QToolButton *tbPaste;

	private slots:
		void loadMenu();
		void saveMenu();
		void loadMenuSelected(QAction* action);
		void saveMenuSelected(QAction* action);
		void saveNewSelected(QString filename);
		void saveDefaults();

	signals:
		void loadConfigRequested(KConfig&);
		void saveConfigRequested(KConfig&);
};

#endif