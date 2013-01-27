#include <stdio.h>

#include <iostream>
#include <vector>

#include <QtSql/QtSql>
#include <QMessageBox>
#include <QInputDialog>
#include <QtXml>

#include "main.h"
#include "sftags-wnd.h"
#include "file.h"

#ifdef SFTAGS_DEBUG
  #include <iostream>
  using std::cout;
  using std::endl;
#endif

using namespace Ui;
using std::vector;

#define XML_FNAME ".sftags.xml"

QApplication *g_app = NULL;
QSettings g_settings("clapautius", "sftags");
FilesAndTagsWnd *gp_main_wnd = NULL;
QString g_xml_path; // :fixme: to be removed - put this in g_settings


bool read_xml(QDomDocument &doc)
{
    QFile file(g_xml_path);
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



/**
 * Helper function used by save_xml().
 * Rename the current xml file to <orig_name>.xxx.
 * @return true if ok, false on error.
 **/
static bool create_backup(QString path)
{
    char suffix[4] = {0};
    for (int i = 0; i < 999; i++) {
        snprintf(suffix, 4, "%03d", i);
        QString backup_path = path + "." + suffix;
        if (!QFile::exists(backup_path)) {
#ifdef SFTAGS_DEBUG
            cout<<":debug: creating backup file "<<Q_STR(backup_path)<<endl;
#endif
            if (rename(Q_STR(path), Q_STR(backup_path)) == 0) {
                return true;
            } else {
                return false;
            }
        }
    }
    // too many backup
    return false;
}


/**
 * Save the current files and tags to the XML config file.
 *
 * @return true if OK, false on error.
 * @todo create a backup first. :fixme:
 **/
bool save_xml(bool first_time)
{
    // create backup
    if (!first_time) {
        if (!create_backup(g_xml_path)) {
            QMessageBox::information(NULL, "Warning", "Cannot create backup XML file");
        }
    }
    
    QFile file(g_xml_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    QString xml_string = get_xml_dump();
    if (file.write(Q_STR(xml_string)) != xml_string.size()) {
#ifdef SFTAGS_DEBUG
        cout<<"Error writing data to xml file"<<endl;
        cout<<"xml_string="<<Q_STR(xml_string)<<endl;
#endif
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

    // check if xml file exists (first time run)
    g_xml_path = QDir::home().path() + "/" + XML_FNAME;
    QFile xml_file(g_xml_path);
    if (!xml_file.exists()) {
        // first run
        QMessageBox::information(NULL, "Info", "First run - creating config. files");
        save_xml(true);
    }

    if (read_xml(xml_doc)) {
        if (parse_xml(xml_doc)) {
            gp_main_wnd = new FilesAndTagsWnd();
            gp_main_wnd->exec();
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
