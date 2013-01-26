#ifndef FILES_TAGS_H
#define FILES_TAGS_H

#include <QApplication>
#include <QString>

#define FILES_TAGS_VER "0.1" // :release:

const char* qstring2c_str(const QString &str);

extern QApplication *g_app;

/**
 * @brief Save the current files and tags to the XML config file.
 **/
bool save_xml();

#endif
