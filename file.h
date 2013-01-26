#ifndef FILE_H
#define FILE_H

#include <map>
#include <set>

#include <QString>
#include <QDomDocument>

class File
{
public:

    File();
    
    File(const QString &path);

    bool has_tags() const;

    const QString path() const;

    void set_path(const QString &path);

    std::set<QString> get_tags() const;

    void set_tags(const std::set<QString>&);

    void add_tag(const QString&);

    void add_file_to_xml_dom(QDomDocument &doc) const;

private:
    QString m_full_path;

    std::set<QString> m_tags;
};

//extern std::map<QString, File> g_files;

// List with all the tags used (by all the files).
//extern std::set<QString> g_tags;

bool get_file_for_path(const QString &path, File &file);
void add_file(const File &file);
void erase_file(const File &file);
QString get_xml_dump();

const std::set<QString> &get_all_used_tags();
void add_used_tag(const QString&);

#endif
