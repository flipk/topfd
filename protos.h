
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
           fdinfo::fdtype _type, netinfo * _ni, int _inode)
        : link(_l), fd(_fd), type(_type), ni(_ni), inode(_inode) { }
    std::string  link;
    uint32_t fd;
    fdtype type;
    netinfo *ni; // used by SOCKET type
    int inode; // used for SOCKET and PIPE
};
typedef std::vector<fdinfo> fdinfo_list;

struct procinfo {
    procinfo(uint32_t _pid, const std::string &_comm)
        : pid(_pid), comm(_comm) { }
    uint32_t pid;
    std::string comm;
    std::string cmdline;
    fdinfo_list fds;
};
typedef std::vector<procinfo> procinfo_list;

struct netinfo_pipe : public netinfo
{
    struct fdinfo_inds {
        int proc_ind;
        int fd;
    };
    procinfo_list &procs;
    std::vector<fdinfo_inds>  proc_inds;
    netinfo_pipe(procinfo_list &_procs) : netinfo("pipe"), procs(_procs) { }
    virtual bool parse(const stringVector &fields) { return true; }
    // fdinfo.link is updated after the fact once all pipes have been
    // discovered, so this doesn't have to do anything.
    virtual std::string _info(void) const { return ""; }
    void add_proc_fd(int _pi, int _fd) {
        fdinfo_inds fdi;
        fdi.proc_ind = _pi;
        fdi.fd = _fd;
        proc_inds.push_back(fdi);
    }
};

stringVector splitstring( const std::string &line );
void printVec(const std::string &input, const stringVector &res); // debug
bool walknet(netinfo_list &netsocks);
bool walkproc(procinfo_list &procs, netinfo_list &netsocks);
fdinfo::fdtype studylink(std::string &link, int &inode,
                         const netinfo_list &netsocks);
