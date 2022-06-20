#include"DataConvert.h"

int main()
{
    DataConvert tool;
    //tool.GenerateDatFile("./location_iwen-100w.txt", "./output-100w.dat");
    //tool.GenerateDatFile("./location_iwen-200w.txt", "./output-200w.dat");
    tool.GenerateDatFileIpv6("./2022-06-20_144835-ipv6.txt", "./output-geoout02.dat");
    //tool.RestorDatToTxt();
    //char buf[256]={0};
    //sprintf(buf, "num:%d\n1234", 100);
    //printf("%s\n",buf);
    //sprintf(buf, "n:%d\n1234", 100);
    //printf("%s\n",buf);
    //tool.GenerateDatFile("./test.txt", "./feng.dat");
    //printf("%u\n",sizeof(IndexInfo));

}
