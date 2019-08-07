
#include "protos.h"

bool
walkproc(procinfo_list &procs, const netinfo_list &netsocks)
{
    pxfe_readdir   procd;

    pxfe_errno e;
    if (procd.open("/proc", &e) == false)
    {
        printf("opendir: %s\n", e.Format().c_str());
        return false;
    }

    dirent de;
    while (procd.read(de))
    {
        std::string pidname = de.d_name;
        uint32_t  pid;
        // many entries in /proc are not numbers. skip them.
        if (pxfe_utils::parse_number(pidname, &pid))
        {
            // an error opening anything in the proc's dir could
            // easily be because of a race where the pid just exited,
            // or because we are running as non-root and don't have
            // access to another userid's pids; so dont bother
            // printing an error if we can't open it.

            std::string procpath = "/proc/" + pidname;
            std::string comm;

            pxfe_fd  commfd;
            if (commfd.open(procpath + "/comm", O_RDONLY) == false)
                continue;
            if (commfd.read(comm, /*max*/ 256) == false)
                continue;
            commfd.close();

            // comm usually has a trailing newline
            if (comm[comm.size()-1] == 10)
                comm.resize(comm.size()-1);

            procs.push_back(procinfo(pid, comm));
            procinfo &pi = procs.back();

            std::string fdpath = procpath + "/fd";
            pxfe_readdir fdd;
            if (fdd.open(fdpath, &e))
            {
                while (fdd.read(de))
                {
                    uint32_t fd;
                    if (pxfe_utils::parse_number(de.d_name, &fd))
                    {
                        std::string fullpath = fdpath + "/" + de.d_name;
                        std::string link;
                        link.resize(300);
                        ssize_t s = readlink(fullpath.c_str(),
                                             (char*) link.c_str(),
                                             link.size());
                        if (s > 0)
                        {
                            link.resize(s);
                            netinfo * ni = NULL;
                            fdinfo::fdtype type = studylink(link, &ni,
                                                            netsocks);
                            pi.fds.push_back(fdinfo(link, fd, type, ni));
                        }
                    }
                }
            }
        }
    }
        // xxxxx match up all PIPEs to each other (and remember
        //       there could be more than one if a fork)
    return true;
}
