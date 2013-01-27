#include <iostream>
#include <vector>

#include <QtXml>

#include "file.h"
#include "main.h"
#include "sftags-wnd.h"

#ifdef SFTAGS_DEBUG
  #include <iostream>
  using std::cout;
  using std::endl;
#endif

extern FilesAndTagsWnd *gp_main_wnd;

using std::set;
using std::vector;

std::map<QString, File> g_files;

// List with all the tags used (by all the files).
std::set<QString> g_tags;

File::File()
{
}


File::File(const QString &path)
{
    m_full_path = path;
}


bool File::has_tags() const
{
    return !m_tags.empty();
}


const QString File::path() const
{
    return m_full_path;
}


void File::set_path(const QString &path)
{
    m_full_path = path;
}


std::set<QString> File::get_tags() const
{
    return m_tags;
}


void File::set_tags(const set<QString> &new_tags)
{
    m_tags = new_tags;
}


void File::add_tag(const QString &new_tag)
{
    m_tags.insert(new_tag);
}


bool File::has_tag(const QString &tag) const
{
    if (m_tags.find(tag) == m_tags.end()) {
        return false;
    } else {
        return true;
    }
}


void File::add_file_to_xml_dom(QDomDocument &doc) const
{
    // add the <file> element
    QDomElement file_elt = doc.createElement("file");
    doc.documentElement().appendChild(file_elt);
    // add the <path> element
    QDomElement path_elt = doc.createElement("path");
    file_elt.appendChild(path_elt);
    QDomText path_text = doc.createTextNode(m_full_path);
    path_elt.appendChild(path_text);
    // add the <tag>s elements
    set<QString>::const_iterator it = m_tags.begin();
    for (; it != m_tags.end(); it++) {
        QDomElement tag_elt = doc.createElement("tag");
        file_elt.appendChild(tag_elt);
        QDomText tag_text = doc.createTextNode(*it);
        tag_elt.appendChild(tag_text);
    }
}


bool get_file_for_path(const QString &path, File &file)
{
    std::map<QString, File>::iterator it;
    it = g_files.find(path);
    if (it == g_files.end()) {
        return false;
    } else {
        file = it->second;
        return true;
    }
}


void add_file(const File &file)
{
    std::cout<<"New file to interesting files list, path="
             <<Q_STR(file.path())<<std::endl;
    g_files[file.path()] = file;
}


void erase_file(const File &file)
{
    g_files.erase(file.path());
}


const std::set<QString>& get_all_used_tags()
{
    return g_tags;
}


void add_used_tag(const QString &new_tag)
{
    if (g_tags.find(new_tag) == g_tags.end()) {
        g_tags.insert(new_tag);
        if (gp_main_wnd) {
            gp_main_wnd->add_used_tag(new_tag);
        }
    }
#ifdef SFTAGS_DEBUG
    else {
        cout<<"Tag "<<Q_STR(new_tag)<<" already exists in the tags list"<<endl;
    }
#endif
}


QString get_xml_dump()
{
    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction(
                      "xml", "version=\"1.0\" encoding=\"UTF-8\""));
    // add root
    QDomElement root = doc.createElement("files");
    doc.appendChild(root);
    // add files
    std::map<QString, File>::const_iterator it = g_files.begin();
    for (; it != g_files.end(); it++) {
        it->second.add_file_to_xml_dom(doc);
    }
    return doc.toString();
}


/**
 * @param[in] p_predicate : poor man's lambda expression ;)
 **/
vector<QString> filter_files(bool (*p_predicate)(const File &, void *),
                             void *p_data)
{
    vector<QString> result;
    std::map<QString, File>::const_iterator it = g_files.begin();
    for (; it != g_files.end(); it++) {
        if (p_predicate(it->second, p_data)) {
            result.push_back(it->first);
        }
    }
    return result;
}
