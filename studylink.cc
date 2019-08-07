
#include "protos.h"

fdinfo::fdtype
studylink(std::string &link, int &inode, const netinfo_list &netsocks)
{
    if (link.size() == 0)
    {
        link = "NULL";
        return fdinfo::NOTSET;
    }
    if (link[0] == '/')
    {
        return fdinfo::FILE;
    }
    if (link.compare(0, 7, "socket:") == 0)
    {
        inode = atoi(link.c_str() + 8);
        return fdinfo::SOCKET;
    }
    if (link.compare(0, 11, "anon_inode:") == 0)
    {
        link.insert(0, "ANON: ");
        return fdinfo::ANON;
    }
    if (link.compare(0, 5, "pipe:") == 0)
    {
        inode = atoi(link.c_str() + 6);
        link = "";
        return fdinfo::PIPE;
    }
    link.insert(0, "UNKNOWN: ");
    return fdinfo::UNKNOWN;
}
