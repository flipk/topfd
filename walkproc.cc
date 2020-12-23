
#include "protos.h"

bool
walkproc(procinfo_list &procs, netinfo_list &netsocks)
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

            {
                pxfe_fd  commfd;
                if (commfd.open(procpath + "/comm", O_RDONLY) == false)
                    continue;
                if (commfd.read(comm, /*max*/ 256) == false)
                    continue;
            }

            // comm usually has a trailing newline
            if (comm[comm.size()-1] == 10)
                comm.resize(comm.size()-1);

            size_t procinfo_ind = procs.size();
            procs.push_back(procinfo(pid, comm));
            procinfo &pi = procs.back();

            {
                pxfe_fd cmdlinefd;
                if (cmdlinefd.open(procpath + "/cmdline", O_RDONLY))
                {
                    if (cmdlinefd.read(pi.cmdline, 4096) == false)
                        pi.cmdline = pi.comm;
                }
                else
                    pi.cmdline = pi.comm;
            }

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
                            int inode = -1;
                            fdinfo::fdtype type = studylink(link, inode,
                                                            netsocks);
                            if (inode != -1)
                            {
                                netinfo_list::const_iterator it =
                                    netsocks.find(inode);

                                if (it == netsocks.end())
                                {
                                    if (type == fdinfo::PIPE)
                                    {
                                        ni = new netinfo_pipe(procs);
                                        ni->inode = inode;
                                        netsocks[inode] = ni;
                                    }
                                }
                                else
                                    ni = it->second;
                                if (ni)
                                    link += " " + ni->info();
                            }
                            pi.fds.push_back(fdinfo(link, fd,
                                                    type, ni, inode));

                            if (type == fdinfo::PIPE)
                            {
                                netinfo_pipe * p =
                                    dynamic_cast<netinfo_pipe*>(ni);
                                if (p)
                                    p->add_proc_fd(procinfo_ind, fd);
                            }
                        }
                    }
                }
            }
        }
    }
    for (size_t pind = 0; pind < procs.size(); pind++)
    {
        procinfo &pi = procs[pind];

        for (size_t fdind = 0; fdind < pi.fds.size(); fdind++)
        {
            fdinfo &fi = pi.fds[fdind];
            if (fi.type == fdinfo::PIPE)
            {
                netinfo_pipe *np = dynamic_cast<netinfo_pipe *>(fi.ni);
                std::ostringstream s;
                if (np)
                {
                    for (size_t pfdind = 0;
                         pfdind < np->proc_inds.size();
                         pfdind++)
                    {
                        netinfo_pipe::fdinfo_inds &fdii =
                            np->proc_inds[pfdind];
                        procinfo &foundpi = procs[fdii.proc_ind];
                        if (foundpi.pid == pi.pid)
                        {
                            if (fi.fd != fdii.fd)
                                s << "(self fd " << fdii.fd << ") ";
                        }
                        else
                            s << "(pid " << foundpi.pid
                              << ", fd " << fdii.fd << ") ";
                    }
                    fi.link += " " + s.str();
                }
            }
        }
    }
    return true;
}
