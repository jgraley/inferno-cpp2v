#ifndef MAGIC_HPP
#define MAGIC_HPP 

// Magic number checks for memory corruption

class Magic   
{
    unsigned magic;
public:
    Magic() :
        magic(0x12343210)
    {    
    }    

    ~Magic()
    {
        ASSERT(magic==0x12343210)("magic number check failed");
        magic = 0xde1e7ed;
    }    
};

#endif
