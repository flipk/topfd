
#include "protos.h"
#include <fstream>
#include <sstream>

// icmp:
//  sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode ref pointer drops             

// icmp6:
//   sl  local_address                         remote_address                        st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode ref pointer drops

// netlink
// sk               Eth Pid        Groups   Rmem     Wmem     Dump  Locks    Drops    Inode

// packet
// sk       RefCnt Type Proto  Iface R Rmem   User   Inode
// 00000000f36aa1b9 3      3    0003   2     1 0      0      10234267

// raw:
//   sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode ref pointer drops
//    1: 00000000:0001 00000000:0000 07 00000000:00000000 00:00000000 00000000  1000        0 10229654 2 00000000d959a5d5 0

// raw6:
//   sl  local_address                         remote_address                        st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode ref pointer drops

// tcp:
//   sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode                                                     
//    0: 00000000:445C 00000000:0000 0A 00000000:00000000 00:00000000 00000000  1000        0 2720575 1 0000000035f8f38e 100 0 0 10 0                   

struct netinfo_tcp : public netinfo {
    std::string local_addr;
    std::string remote_addr;
    netinfo_tcp(void) : netinfo("tcp") { }
    virtual bool parse(const stringVector &fields) {
        if (fields.size() > 15)
        {
            local_addr = fields[1];
            remote_addr = fields[2];
            uid = atoi(fields[7].c_str());
            inode = atoi(fields[9].c_str());
            return true;
        }
        return false;
    }
    virtual std::string _info(void) const {
        std::ostringstream s;
        s << "local:" << local_addr << " remote:" << remote_addr;
        return s.str();
    }
};

// tcp6:
//   sl  local_address                         remote_address                        st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode

// udp:
//   sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode ref pointer drops             
//  1512: 00000000:0044 00000000:0000 07 00000000:00000000 00:00000000 00000000     0        0 26498 2 000000006ef5c200 0         

struct netinfo_udp : public netinfo {
    std::string local_addr;
    std::string remote_addr;
    netinfo_udp(void) : netinfo("udp") { }
    virtual bool parse(const stringVector &fields) {
        if (fields.size() > 9)
        {
            local_addr = fields[1];
            remote_addr = fields[2];
            uid = atoi(fields[7].c_str());
            inode = atoi(fields[9].c_str());
            return true;
        }
        return false;
    }
    virtual std::string _info(void) const {
        std::ostringstream s;
        s << "local:" << local_addr << " remote:" << remote_addr;
        return s.str();
    }
};

// udp6:
//   sl  local_address                         remote_address                        st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode ref pointer drops

// udplite
//   sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode ref pointer drops             

// udplite6
//   sl  local_address                         remote_address                        st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode ref pointer drops

// unix
// Num       RefCount Protocol Flags    Type St Inode Path
// 00000000ba68e9a4: 00000003 00000000 00000000 0001 03 32632 /run/dbus/system_bus_socket

struct netinfo_unix : public netinfo {
    std::string path;
    netinfo_unix(void) : netinfo("unix") { }
    virtual bool parse(const stringVector &fields) {
        if (fields.size() > 6)
        {
            inode = atoi(fields[6].c_str());
            if (fields.size() > 7)
                path = fields[7];
            else
                path = "(anonymous)";
            return true;
        }
        return false;
    }
    virtual std::string _info(void) const {
        return path;
    }
};

#define PROTOLIST \
    PROTOITEM(tcp) \
    PROTOITEM(udp) \
    PROTOITEM(unix)

//static
netinfo *
netinfo::make(const std::string &name)
{
#define PROTOITEM(str) if (name == #str) return new netinfo_##str;
    PROTOLIST;
#undef  PROTOITEM
    return NULL;
}

std::string
netinfo::info(void) const
{
    std::ostringstream s;
//    s << "inode:" << inode;
    s << proto;
    if (uid != -1)
        s << " uid:" << uid;
    s << " " << _info();
    return s.str();
}

static const std::string supported_protos[] = {
#define PROTOITEM(str)  #str ,
    PROTOLIST
#undef  PROTOITEM
    ""
};

bool
walknet(netinfo_list &netsocks)
{
    std::string line;

    for (int protoind = 0; supported_protos[protoind].size() > 0; protoind++)
    {
        const std::string &name = supported_protos[protoind];

        const std::string filename = "/proc/net/" + name;

        std::ifstream inf(filename.c_str());
        if (!inf.good())
        {
            int e = errno;
            printf("unable to open '%s': %d (%s)\n",
                   filename.c_str(), e, strerror(e));
            continue;
        }

        int line_number = 1;
        getline(inf, line);
        while (inf.good())
        {
            stringVector  sv;
            sv = splitstring(line);
            if (sv.size() > 0)
            {
                if (line_number == 1)
                {
//                    printf("header line: '%s'\n", line.c_str());
                }
                else
                {
//                    printVec(line, sv);
                    netinfo * ni = netinfo::make(name);
                    if (ni)
                    {
                        if (ni->parse(sv))
                        {
                            std::string info = ni->info();
                            netsocks[ni->inode] = ni;
                        }
                        else
                            delete ni;
                    }
                }
            }
            getline(inf, line);
            line_number ++;
        }
    }
    return true;
}
