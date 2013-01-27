#ifndef FILES_TAGS_H
#define FILES_TAGS_H

#include <QApplication>
#include <QString>

#define SFTAGS_VER "0.1" // :release:

#define SFTAGS_DEBUG

#define Q_STR(x) (x).toUtf8().constData()

extern QApplication *g_app;

/**
 * @brief Save the current files and tags to the XML config file.
 **/
bool save_xml(bool first_time = false);

#endif
