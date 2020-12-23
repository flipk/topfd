
#include "protos.h"

int
main()
{
    netinfo_list   netsocks;
    procinfo_list  procs;

    if (walknet(netsocks) == false)
    {
        printf("FAIL\n");
        return 1;
    }
#if 0 // DEBUG
    {
        netinfo_list::iterator  it;
        for (it = netsocks.begin();
             it != netsocks.end();
             it++)
        {
            netinfo * ni = it->second;
            printf("%s\n", ni->info().c_str());
        }
    }
#endif
    if (walkproc(procs, netsocks) == false)
    {
        printf("FAIL\n");
        return 1;
    }
    printf("procs:\n");
    for (size_t pind = 0; pind < procs.size(); pind++)
    {
        const procinfo &pi = procs[pind];
        if (pi.fds.size() == 0)
            continue;
        printf("fds for pid %u (%s) (%s):\n",
               pi.pid, pi.comm.c_str(), pi.cmdline.c_str());
        for (size_t ind = 0; ind < pi.fds.size(); ind++)
        {
            const fdinfo &fd = pi.fds[ind];
            printf("%u : %s\n", fd.fd, fd.link.c_str());
        }
    }
    return 0;
}
