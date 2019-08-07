
#include "protos.h"

fdinfo::fdtype
studylink(std::string &link, netinfo **nip, const netinfo_list &netsocks)
{
    if (link.size() == 0)
    {
        link = "NULL";
        return fdinfo::NOTSET;
    }
    if (link[0] == '/')
    {
        link.insert(0, "FILE: ");
        return fdinfo::FILE;
    }
    if (link.compare(0, 7, "socket:") == 0)
    {
        int inode = atoi(link.c_str() + 8);
        link.insert(0, "SOCKET: ");
        netinfo_list::const_iterator it = netsocks.find(inode);
        if (it != netsocks.end())
        {
            const netinfo * ni = it->second;
            link += " " + ni->info();
        }
        return fdinfo::SOCKET;
    }
    if (link.compare(0, 11, "anon_inode:") == 0)
    {
        link.insert(0, "ANON: ");
        return fdinfo::ANON;
    }
    if (link.compare(0, 5, "pipe:") == 0)
    {
        link.insert(0, "PIPE: ");
        return fdinfo::PIPE;
    }
    link.insert(0, "UNKNOWN: ");
    return fdinfo::UNKNOWN;
}
