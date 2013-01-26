#ifndef FILES_TAGS_H
#define FILES_TAGS_H

#include <QApplication>
#include <QString>

#define FILES_TAGS_VER "0.1" // :release:

const char* qstr2cchar(const QString &str);

extern QApplication *g_app;

//bool contains(std::vector<QString> vect, QString elt);

#endif
