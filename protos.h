
#include "../pfkutils/libpfkutil/posix_fe.h"
#include <inttypes.h>
#include <string>
#include <vector>
#include <map>

typedef std::vector<std::string> stringVector;

struct netinfo {
    int inode;
    int uid;
    std::string proto;
    virtual bool parse(const stringVector &fields) = 0;
    virtual std::string _info(void) const = 0;
    std::string info(void) const;
    static netinfo * make(const std::string &name);
    netinfo(const std::string &_proto) : inode(-1), uid(-1), proto(_proto) { }
};
typedef std::map<int /*inode*/, netinfo*> netinfo_list;

struct fdinfo {
    typedef enum {
        NOTSET, FILE, SOCKET, ANON, PIPE, UNKNOWN
    } fdtype;
    fdinfo(const std::string &_l, uint32_t _fd,
           fdinfo::fdtype _type, netinfo * _ni)
        : link(_l), fd(_fd), type(_type), ni(_ni) { }
    std::string  link;
    uint32_t fd;
    fdtype type;
    netinfo *ni;
};
typedef std::vector<fdinfo> fdinfo_list;

struct procinfo {
    procinfo(uint32_t _pid, const std::string &_comm)
        : pid(_pid), comm(_comm) { }
    uint32_t pid;
    std::string comm;
    fdinfo_list fds;
};
typedef std::vector<procinfo> procinfo_list;

stringVector splitstring( const std::string &line );
void printVec(const std::string &input, const stringVector &res); // debug
bool walknet(netinfo_list &netsocks);
bool walkproc(procinfo_list &procs, const netinfo_list &netsocks);
fdinfo::fdtype studylink(std::string &link, netinfo **nip,
                         const netinfo_list &netsocks);
