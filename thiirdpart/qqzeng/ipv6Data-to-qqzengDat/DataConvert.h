#include<string>
#include<stdint.h>
#include<map>
#include<vector>
using namespace std;

#pragma pack(push, 1)
//struct IndexInfo
//{
//    uint32_t endip;
//    uint32_t dataoffset:24;
//    uint32_t datalen:8;
//};

struct IndexInfo
{
    uint32_t endip;
    uint32_t dataoffset;
    uint16_t datalen;
};


struct PrefixInfo
{
    //uint8_t prefix_value;
    uint32_t low;
    uint32_t high;
};

struct PrefixInfoWrp
{

    uint32_t pre;
    PrefixInfo preinfo;
};

struct st_DatFile_Head
{
    uint32_t index_first;
    uint32_t index_end;
    uint32_t prefix_fist;
    uint32_t prefix_end;
};
#pragma pack(pop)

struct st_DataInfo
{
    uint32_t dataoffset;
    uint32_t datalen;
};

typedef multimap<uint8_t, IndexInfo> mymap;
class IPSearch;
class DataConvert
{
    public:
        int GenerateDatFile(const char*, const char*);
        int RestorDatToTxt();
        int GenerateDatFileIpv6(const char*, const char*);
    private:
        int FillIndexArea(int);
    private:
        FILE* m_outFd;
        mymap m_map;
        int ipElemNums;
        //st_DatFile_Head stDatFileHead;
        vector<PrefixInfoWrp> m_prefixVec;
        IPSearch *m_qqzeng;
        map<uint32_t,st_DataInfo> m_hashmap;

};
