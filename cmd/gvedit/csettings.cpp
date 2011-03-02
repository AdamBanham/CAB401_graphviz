/* $Id$Revision: */
/* vim:set shiftwidth=4 ts=8: */

/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: See CVS logs. Details at http://www.graphviz.org/
 *************************************************************************/

#include "csettings.h"
#include "qmessagebox.h"
#include "qfiledialog.h"
#include <QtGui>
#include <qfile.h>
#include "mdichild.h"
#include "string.h"
#define WIDGET(t,f)  ((t*)findChild<t *>(#f))


bool loadAttrs(const QString fileName,QComboBox* cbNameG,QComboBox* cbNameN,QComboBox* cbNameE)
{
    QStringList lines;
    QFile file(fileName);
    if ( file.open(QIODevice::ReadOnly ) ) {
        QTextStream stream( &file );
        QString line;
        int i = 1;
        while ( !stream.atEnd() ) {
            line = stream.readLine(); // line of text excluding '\n'
	    if(line.left(1)==":")
	    {
		QString attrName;
		QStringList sl= line.split(":");
		for (int id=0;id < sl.count(); id ++)
		{
		    if(id==1)
			attrName=sl[id];
		    if(id==2)
		    {
			if(sl[id].contains("G"))
			    cbNameG->addItem(attrName);
			if(sl[id].contains("N"))
			    cbNameN->addItem(attrName);
			if(sl[id].contains("E"))
			    cbNameE->addItem(attrName);
		    }
		    printf ("%s\n",sl[id].constData());
		};
	    }
        }
        file.close();
    }
    return false;




}
QString stripFileExtension(QString fileName)
{
    int idx;
    for (idx=fileName.length();idx >=0 ; idx --)
    {
	if(fileName.mid(idx,1)==".")
	    break;
    }
    return fileName.left(idx);
}


char* graph_reader( char * str, int num, FILE * stream ) //helper function to load / parse graphs from tstring
{
    if (num==0)
	return str;
    char* ptr;
    int l=0;
    CFrmSettings* s=reinterpret_cast<CFrmSettings*>(stream); //as ugly as it gets :)
    if(s->cur >=strlen(s->graphData.toUtf8().constData()))
	return NULL;
    strcpy(str,(char*)s->graphData.mid(s->cur,num-1).toUtf8().constData());
    ptr = strchr(str,'\n');
    if (ptr) {
	ptr++;
	*ptr = '\0';
	l = ptr - str;
    }
    else
	l=strlen (str);
    s->cur += l;
    return str;

}


CFrmSettings::CFrmSettings()
{
    this->gvc=gvContext();
    Ui_Dialog tempDia;
    tempDia.setupUi(this);
    graph=NULL;

    connect(WIDGET(QPushButton,pbAdd),SIGNAL(clicked()),this,SLOT(addSlot()));
    connect(WIDGET(QPushButton,pbNew),SIGNAL(clicked()),this,SLOT(newSlot()));
    connect(WIDGET(QPushButton,pbOpen),SIGNAL(clicked()),this,SLOT(openSlot()));
    connect(WIDGET(QPushButton,pbSave),SIGNAL(clicked()),this,SLOT(saveSlot()));
    connect(WIDGET(QPushButton,btnOK),SIGNAL(clicked()),this,SLOT(okSlot()));
    connect(WIDGET(QPushButton,btnCancel),SIGNAL(clicked()),this,SLOT(cancelSlot()));
    connect(WIDGET(QPushButton,pbOut),SIGNAL(clicked()),this,SLOT(outputSlot()));
    connect(WIDGET(QComboBox,cbScope),SIGNAL(currentIndexChanged(int)),this,SLOT(scopeChangedSlot(int)));


    loadAttrs("c:/graphviz-ms/bin/attrs.txt",WIDGET(QComboBox,cbNameG),WIDGET(QComboBox,cbNameN),WIDGET(QComboBox,cbNameE));
}

void CFrmSettings::outputSlot()
{
    QString _filter="Output File(*."+WIDGET(QComboBox,cbExtension)->currentText()+")";
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Graph As.."),"/",_filter);
 if (!fileName.isEmpty())
     WIDGET(QLineEdit,leOutput)->setText(fileName);
}
void CFrmSettings::scopeChangedSlot(int id)
{
    WIDGET(QComboBox,cbNameG)->setVisible(id==0);
    WIDGET(QComboBox,cbNameN)->setVisible(id==1);
    WIDGET(QComboBox,cbNameE)->setVisible(id==2);
}
void CFrmSettings::addSlot()
{
    QString _scope=WIDGET (QComboBox,cbScope)->currentText();
    QString _name;
    switch (WIDGET  (QComboBox,cbScope)->currentIndex())
    {
    case 0:
	_name=WIDGET  (QComboBox,cbNameG)->currentText();
	break;
    case 1:
	_name=WIDGET  (QComboBox,cbNameN)->currentText();
	break;
    case 2:
	_name=WIDGET  (QComboBox,cbNameE)->currentText();
	break;
    }
    QString _value=WIDGET(QLineEdit,leValue)->text();

    if (_value.trimmed().length() == 0)
	QMessageBox::warning(this, tr("GvEdit"),tr("Please enter a value for selected attribute!"),QMessageBox::Ok,QMessageBox::Ok);
    else
    {
	QString str=_scope+"["+_name+"=\"";
	if(WIDGET (QTextEdit,teAttributes)->toPlainText().contains(str))
	{
	    QMessageBox::warning(this, tr("GvEdit"),tr("Attribute is already defined!"),QMessageBox::Ok,QMessageBox::Ok);
	    return;
	}
	else
	{
	    str = str + _value+"\"]";
	    WIDGET (QTextEdit,teAttributes)->setPlainText(WIDGET (QTextEdit,teAttributes)->toPlainText()+str+"\n");

	}
    }
}
void CFrmSettings::helpSlot(){}
void CFrmSettings::cancelSlot()
{
    this->reject();
}
void CFrmSettings::okSlot()
{
    saveContent();
    this->done(drawGraph());
}
void CFrmSettings::newSlot()
{
    WIDGET (QTextEdit,teAttributes)->setPlainText(tr(""));
}
void CFrmSettings::openSlot()
{
 QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"/",tr("Text file (*.*)"));
 if (!fileName.isEmpty())
 {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("MDI"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    WIDGET (QTextEdit,teAttributes)->setPlainText(in.readAll());
 }

}
void CFrmSettings::saveSlot(){

    if(WIDGET (QTextEdit,teAttributes)->toPlainText().trimmed().length()==0)
    {
	QMessageBox::warning(this, tr("GvEdit"),tr("Nothing to save!"),QMessageBox::Ok,QMessageBox::Ok);
	return;


    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Open File"),
                                                "/",
                                                 tr("Text File(*.*)"));
 if (!fileName.isEmpty())
 {

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("MDI"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    out << WIDGET (QTextEdit,teAttributes)->toPlainText();
    return;
 }

}

QString CFrmSettings::buildOutputFile(QString _fileName)
{
    return QString("sfsdfdf");
}

void CFrmSettings::addAttribute(QString _scope,QString _name,QString _value){}
bool CFrmSettings::loadGraph(MdiChild* m)
{
    cur=0;
    if(graph)
	agclose(graph);
    graphData.clear();
    graphData.append(m->toPlainText());
    setActiveWindow(m);
    return true;

}
bool CFrmSettings::createLayout()
{
    //first attach attributes to graph
    int _pos=graphData.indexOf(tr("{"));
    graphData.replace(_pos,1,"{"+WIDGET(QTextEdit,teAttributes)->toPlainText());

      /* Reset line number and file name;
       * If known, might want to use real name
       */
    agsetfile("<gvedit>");
    cur=0;
    graph=agread_usergets(reinterpret_cast<FILE*>(this),(gets_f)graph_reader);
    if(!graph)
	return false;
    Agraph_t* G=this->graph;
    gvLayout (gvc, G, (char*)WIDGET(QComboBox,cbLayout)->currentText().toUtf8().constData()); /* library function */
    return true;
}
bool CFrmSettings::renderLayout()
{
    if(graph)
    {
	QString _fileName(WIDGET(QLineEdit,leOutput)->text());
	_fileName=stripFileExtension(_fileName);
	_fileName=_fileName+"."+WIDGET(QComboBox,cbExtension)->currentText();
	int rv=gvRenderFilename(gvc,graph,(char*)WIDGET(QComboBox,cbExtension)->currentText().toUtf8().constData(),(char*)_fileName.toUtf8().constData());
	this->getActiveWindow()->loadPreview(_fileName);
	if(rv)
	    this->getActiveWindow()->loadPreview(_fileName);
	return rv;

    }
    return false;
}



bool CFrmSettings::loadLayouts()
{
    return false;
}

bool CFrmSettings::loadRenderers()
{
    return false;
}

void CFrmSettings::refreshContent()
{

    WIDGET(QComboBox,cbLayout)->setCurrentIndex(activeWindow->layoutIdx);
    WIDGET(QComboBox,cbExtension)->setCurrentIndex(activeWindow->renderIdx);
    if(!activeWindow->outputFile.isEmpty())
	WIDGET(QLineEdit,leOutput)->setText(activeWindow->outputFile);
    else
	WIDGET(QLineEdit,leOutput)->setText(stripFileExtension(activeWindow->currentFile())+  "."+WIDGET(QComboBox,cbExtension)->currentText());

    WIDGET(QTextEdit,teAttributes)->setText(activeWindow->attributes);

    WIDGET(QLineEdit,leValue)->setText("");

}

void CFrmSettings::saveContent()
{
    activeWindow->layoutIdx=WIDGET(QComboBox,cbLayout)->currentIndex();
    activeWindow->renderIdx=WIDGET(QComboBox,cbExtension)->currentIndex();
    activeWindow->outputFile=WIDGET(QLineEdit,leOutput)->text();
    activeWindow->attributes=WIDGET(QTextEdit,teAttributes)->toPlainText();
}
int CFrmSettings::drawGraph()
{
	    createLayout();
	    renderLayout();
	    getActiveWindow()->settingsSet=false;
	    return QDialog::Accepted;

}
int CFrmSettings::runSettings(MdiChild* m)
{
    if ((m) && (m==getActiveWindow()))
    {
        if(this->loadGraph(m))
	    return drawGraph();
	else
            return QDialog::Rejected;
    }

    else
	return showSettings(m);

}
int CFrmSettings::showSettings(MdiChild* m)
{

        if(this->loadGraph(m))
	{
	    refreshContent();
	    return this->exec();
	}
	else
	    return QDialog::Rejected;
}

void CFrmSettings::setActiveWindow(MdiChild* m)
{
    this->activeWindow=m;

}
MdiChild* CFrmSettings::getActiveWindow()
{
    return activeWindow;
}



