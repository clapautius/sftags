#include <iostream>
#include <vector>

#include <QtSql/QtSql>
#include <QMessageBox>
#include <QInputDialog>
#include <QtXml>

#include "main.h"
#include "sftags-wnd.h"
#include "file.h"

using namespace Ui;
using std::vector;

#define XML_PATH "sftags.xml"

QApplication *g_app = NULL;

QSettings gSettings("clapautius", "sftags");

const char* qstr2cchar(const QString &str)
{
    return str.toUtf8().constData();
}


bool read_xml(const char *path, QDomDocument &doc)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();
    return true;
}


bool parse_file_entity(QDomNode &node)
{
    File file;
    bool path_ok = false;
    QDomNodeList children = node.childNodes();
    for (int i = 0; i < children.size(); i++) {
        QDomNode node = children.at(i);
        QString name = node.nodeName();
        if (name == "path") {
            path_ok = true;
            file.set_path(node.firstChild().nodeValue());
        } else if (name == "tag") {
            QString tag = node.firstChild().nodeValue();
            file.add_tag(tag);
            add_used_tag(tag);
        }
    }
    if (path_ok) {
        add_file(file);
        return true;
    } else {
        return false;
    }
}


bool parse_xml(QDomDocument &doc)
{
    QDomNodeList files = doc.elementsByTagName("file");
    for (int i = 0; i < files.size(); i++) {
        QDomNode node = files.at(i);
        if (!parse_file_entity(node)) {
            return false;
        }
    }
    return true;
}



bool save_xml(const char *path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    QString xml_string = get_xml_dump();
    if (file.write(qstr2cchar(xml_string)) < 0) {
        file.close();
        return false;
    }
    file.close();
    return true;
}


int main( int argc, char **argv )
{
    g_app=new QApplication( argc, argv );
    QDomDocument xml_doc;
    if (read_xml(XML_PATH, xml_doc)) {
        if (parse_xml(xml_doc)) {
            FilesAndTagsWnd *w = new FilesAndTagsWnd();
            w->exec();
            if (!save_xml(XML_PATH)) {
                QMessageBox::critical(NULL, "Error", "Error saving XML document");
            }
            return 0;
        } else {
            QMessageBox::critical(NULL, "Error", "Error parsing XML document");
            return 1;
        }
    } else {
        QMessageBox::critical(NULL, "Error", "Error reading XML document");
    }
}


bool contains(vector<QString> vect, QString elt)
{
    for (uint i=0; i<vect.size(); i++)
        if (vect[i]==elt)
            return true;
    return false;
}
