
#include "protos.h"

stringVector
splitstring( const std::string &line )
{
    stringVector ret;
    size_t pos;
    for (pos = 0; ;)
    {
        size_t found = line.find_first_of("\t ",pos);
        if (found == std::string::npos)
        {
            if (pos != line.size())
                ret.push_back(line.substr(pos,line.size()-pos));
            break;
        }
        if (found > pos)
            ret.push_back(line.substr(pos,found-pos));
        pos = found+1;
    }
    return ret;
}

void
printVec(const std::string &input, const stringVector &res)
{
    std::cout << "for input '" << input << "' the vector contents are:" << std::endl;
    for (int ind = 0; ind < res.size(); ind++)
    {
        const std::string &s = res[ind];
        std::cout << ind << ": '" << s << "'" << std::endl;
    }
}

#ifdef __INCLUDE_SPLITSTRING_TEST_MAIN__
#include <iostream>

void
testit(const std::string &line)
{
    stringVector  sv = splitstring(line);
    printVec(line, sv);
}

int
main()
{
    testit("");
    testit("one");
    testit("one two three four");
    testit("    one     two     three four");
    return 0;
}
#endif
