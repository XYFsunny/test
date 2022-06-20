#include"DataConvert.h"
#include"IPLocator.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *strchrN(char *str, char flag, int num)
{
    if(0 == num)
        return str;
    char *p;
    p=strchr(str, flag);
    return strchrN(p+1,flag,num - 1);
    
}

int rte_strsplit(char *string, int stringlen,
                char **tokens, int maxtokens, char delim)
{
        int i, tok = 0;
        int tokstart = 1; 

        if (string == NULL || tokens == NULL)
                return -1;

        for (i = 0; i < stringlen; i++) {
                if (string[i] == '\0' || tok >= maxtokens)
                        break;
                if (tokstart) {
                        tokstart = 0;
                        tokens[tok++] = &string[i];
                }
                if (string[i] == delim) {
                        string[i] = '\0';
                        tokstart = 1;
                }
        }
        return tok;
}

uint32_t ipToLong(const char * ip, uint32_t &prefix)
{
        int a, b, c, d;
        int iLen;
        int abcdIndex = 0;
        iLen = strlen(ip);
        char ips[3];
        memset(ips, '\0', 3);

        int ipsCnt = 0;
        for (int i = 0; i < iLen; i++)
        {
                if ('.' == ip[i])
                {
                        if (0 == abcdIndex)
                        {
                                abcdIndex = 1;
                                a = atoi(ips);
                        }
                        else if (1 == abcdIndex)
                        {
                                abcdIndex = 2;
                                b = atoi(ips);
                        }
                        else if (2 == abcdIndex)
                        {
                                abcdIndex = 3;
                                c = atoi(ips);
                        }

                        ipsCnt = 0;
                        memset(ips, '\0', 3);
                }
                else
                {
                        ips[ipsCnt] = ip[i];
                        ipsCnt++;
                }
        }
        d = atoi(ips);

        prefix = (uint32_t)a;
        return ((uint8_t)a << 24) | ((uint8_t)b << 16) | ((uint8_t)c << 8) | (uint8_t)d;

}

unsigned int RSHash( char * str)
{
    unsigned int b = 378551 ;

    unsigned int a = 63689 ;
    unsigned int hash = 0 ;
    while ( * str)
    {
        hash = hash * a + ( * str ++ );
        a *= b;
    }
    return (hash & 0x7FFFFFFF);
}

unsigned int JSHash( char * str)
{
    unsigned int hash = 1315423911;
    while ( * str)
    {
        hash ^= ((hash << 5 ) + ( * str ++ ) + (hash >> 2 ));
    }
    return (hash & 0x7FFFFFFF );
}

//#define BUFSIZE 1024
#define BUFSIZE 65535
#define TOKENNUM 14
#define DAT_HEAD_SIZE 4
int GetIpElemNum(FILE *fd, int &line,int &keynum)
{
    int num = 0;
    char buf[BUFSIZE];
    char *splitstr[TOKENNUM+1];
    int curkey=0,prekey=-1;
    fseek(fd, 0, SEEK_SET);
    while(fgets(buf, sizeof(buf), fd))
    {
        //if(line == 5057643)
        if(strlen(buf)>=255)
        {
            //printf("%s\n", buf);
           // printf("-------------------1-----------------\n");
            //continue;
        }
        
        if((TOKENNUM+1) != rte_strsplit(buf, sizeof(buf), splitstr, TOKENNUM+1, '|'))
        {
            printf("error\n");
            exit(0);
        }
        curkey = atoi(splitstr[0]);
        if(curkey!=prekey)
        {
            prekey = curkey;
            keynum++;
        }

        line++;
    }
    fseek(fd, 0, SEEK_SET);
    return num;

}

int DataConvert::GenerateDatFile(const char* infilename, const char *outfilename)
{

    uint32_t testippre;
    ipToLong("119.2.128.54", testippre);
    //printf("---------:%d\n",testippre);
    //return 0;

    m_outFd = fopen(outfilename, "wb");
    if(NULL == m_outFd)
        return -1;
    FILE *fd = fopen(infilename, "r" );
    if(NULL == fd)
        return -1;

    char buf[BUFSIZE];
    char *splitstr[TOKENNUM+1];
    uint32_t ip_prefix, startip, endip;
    IndexInfo itmp;
    char *filldata;
    uint32_t filllen;

    uint32_t hashvalue;
    map<uint32_t,st_DataInfo>::iterator hashit; 
    st_DataInfo stDataInfo;
    int i=0;
    int ipElemNums=0,keynum=0;
    GetIpElemNum(fd,ipElemNums,keynum);
    fwrite(&ipElemNums, sizeof(ipElemNums), 1, m_outFd);
    //uint32_t uDataAreaOffset = DAT_HEAD_SIZE + 8*256 + ipElemNums*9;
    uint32_t uDataAreaOffset = DAT_HEAD_SIZE + 8*256 + ipElemNums*sizeof(IndexInfo);
    fseek(m_outFd, uDataAreaOffset, SEEK_SET);
    uint32_t curroffset=uDataAreaOffset;

uint32_t numcount = 0,errnum=0;
int diffnum = 0;
uint64_t total_len =0;
    while(fgets(buf, sizeof(buf), fd))
    {

        filldata = strchrN(buf, '|', 4);
        filllen = strlen(filldata) - 1 ;
        total_len+=filllen;

        hashvalue= RSHash(filldata);
        if(numcount == 8924740)
        {

            //printf("%d\n", numcount);
        }
        //printf("num: %d, hashvalue: %u\n", i++, hashvalue);
        if(m_hashmap.end() != (hashit = m_hashmap.find(hashvalue)))
        {
            itmp.dataoffset = hashit->second.dataoffset;
            itmp.datalen = hashit->second.datalen;
        }
        else
        {
                uint32_t rlen = fwrite(filldata, filllen, 1, m_outFd);
                if(1 != rlen)
                {
                        printf("write data error.\n");
                        return -1;
                }

                stDataInfo.dataoffset = curroffset;
                stDataInfo.datalen = filllen;

                itmp.dataoffset = curroffset;
                itmp.datalen = filllen;

                m_hashmap[hashvalue] = stDataInfo;
                curroffset+=filllen;
        //        printf("diffnum:%d \n", diffnum++);

        }


        if((TOKENNUM+1) != rte_strsplit(buf, sizeof(buf), splitstr, TOKENNUM+1, '|'))
        //if( 0>=rte_strsplit(buf, sizeof(buf), splitstr, TOKENNUM+1, '|'))
        {
            errnum++;
            printf("num:%d,errnum:%d, data error.\n",numcount,errnum);
            return -2;
        }

        numcount++;
        //ipToLong(splitstr[0], ip_prefix);
        ip_prefix = atoi(splitstr[0]);
        if(255<ip_prefix)
        {
            static uint32_t f = 0;
            printf("0num:%u\n", f++);
        }
        
        itmp.endip = atoi(splitstr[3]);

        //printf("prefix:%d,  end:%u, offset:%u, len:%d\n", ip_prefix, itmp.endip, itmp.dataoffset,itmp.datalen);
        m_map.insert(make_pair(ip_prefix, itmp));
    }
    printf("totalnum: %u\n", curroffset);
    printf("0totalnum: %u\n", m_map.count(0));
    printf("1totalnum: %u\n", m_map.count(1));
    printf("2totalnum: %u\n", m_map.count(2));
    printf("3totalnum: %u\n", m_map.count(3));
    printf("4totalnum: %u\n", m_map.count(4));
    printf("5totalnum: %u\n", m_map.count(5));
    printf("6totalnum: %u\n", m_map.count(6));
    printf("30totalnum: %u\n", m_map.count(30));

    uint32_t isize = m_map.size();
    printf("size: %u\n", isize);
    //stDatFileHead.index_first = curroffset;
    FillIndexArea(keynum);
    fclose(m_outFd);

}


int DataConvert::FillIndexArea(int keynum)
{
    
    uint8_t ip_prefix;
    IndexInfo iIndexInfo;
    PrefixInfoWrp iPrefixInfo;
    mymap::iterator it = m_map.begin();
    int32_t ip_prefix_last = -1, i, prefix_count;
    fseek(m_outFd, DAT_HEAD_SIZE+8*256, SEEK_SET);
    int p=0;
    //填充索引区域
    for(it,i=0 ; it!=m_map.end(); it++, i++)
    {
        ip_prefix = it->first;
        iIndexInfo = it->second;
        if(1 != fwrite(&iIndexInfo, sizeof(IndexInfo), 1, m_outFd))
        {
            printf("write data error.\n");
            return -1;
        }

        if(ip_prefix != ip_prefix_last)
        {
            ip_prefix_last = ip_prefix;
            prefix_count = m_map.count(ip_prefix);
            iPrefixInfo.pre = ip_prefix;
            iPrefixInfo.preinfo.low = i;
            iPrefixInfo.preinfo.high = i + prefix_count - 1;
            m_prefixVec.push_back(iPrefixInfo);
            //printf("num:%d \n", p++);
        }
    }
    //stDatFileHead.index_end = stDatFileHead.index_first + indexarea_size - sizeof(IndexInfo);
    //stDatFileHead.prefix_fist = stDatFileHead.index_first + indexarea_size;

    //uint32_t prefixarea_size = 0;
    fseek(m_outFd, DAT_HEAD_SIZE, SEEK_SET);
    //填充前缀区域
    int ii=0;
    PrefixInfo tmpPrefixInfo={0,0};
    for(auto &it : m_prefixVec)
    {
            uint32_t tmpPre = it.pre;
            if(tmpPre == ii)
            {
                    if(1 != fwrite(&(it.preinfo), sizeof(PrefixInfo), 1, m_outFd))
                    {
                            printf("write data error.\n");
                            return -1;
                    }
                    printf("prefix:%d, start:%d, end:%d\n", ii, it.preinfo.low, it.preinfo.high);
            }
            else
            {
                    int sub = tmpPre - ii;
                    for(int i=0;i<sub;i++)
                    {
                            if(1 != fwrite(&tmpPrefixInfo, sizeof(PrefixInfo), 1, m_outFd))
                            {
                                    printf("write data error.\n");
                                    return -1;
                            }      
                            printf("prefix:%d, start:%d, end:%d\n", ii+i,0,0); 
                            ii++;
                    }
                    if(1 != fwrite(&(it.preinfo), sizeof(PrefixInfo), 1, m_outFd))
                    {
                            printf("write data error.\n");
                            return -1;
                    }
                    printf("prefix:%d, start:%d, end:%d\n", tmpPre, it.preinfo.low, it.preinfo.high);
            }

            ii++;

            //   prefixarea_size += sizeof(PrefixInfo);
    }
    //stDatFileHead.prefix_end = stDatFileHead.prefix_fist +  prefixarea_size - sizeof(PrefixInfo);

    //fseek(m_outFd, 0, SEEK_SET);

    //fwrite(&stDatFileHead, sizeof(stDatFileHead), 1, m_outFd);
}

int DataConvert::RestorDatToTxt()
{
    m_qqzeng = IPSearch::instance();
    m_qqzeng->printinfo();

}

























//--------------ipv6---------
#define TOKENNUM_IPV6 17
int GetIpElemNumIpv6(FILE *fd, int &line,int &keynum)
{
    int num = 0;
    char buf[BUFSIZE];
    char *splitstr[TOKENNUM_IPV6+1];
    int curkey=0,prekey=-1;
    fseek(fd, 0, SEEK_SET);
    while(fgets(buf, sizeof(buf), fd))
    {
        if(strlen(buf)>=255)
        {
            //printf("%s\n", buf);
           // printf("-------------------1-----------------\n");
            //continue;
        }
        
        if((TOKENNUM_IPV6+1) != rte_strsplit(buf, sizeof(buf), splitstr, TOKENNUM_IPV6+1, ','))
        {
            printf("error\n");
            exit(0);
        }
        curkey = atoi(splitstr[0]);
        if(curkey!=prekey)
        {
            prekey = curkey;
            keynum++;
        }

        line++;
    }
    fseek(fd, 0, SEEK_SET);
    return num;

}
int DataConvert::GenerateDatFileIpv6(const char* infilename, const char *outfilename)
{

    uint32_t testippre;

    m_outFd = fopen(outfilename, "wb");
    if(NULL == m_outFd)
        return -1;
    FILE *fd = fopen(infilename, "r" );
    if(NULL == fd)
        return -1;

    char buf[BUFSIZE];
    char *splitstr[TOKENNUM+1];
    uint32_t ip_prefix, startip, endip;
    IndexInfo itmp;
    char *filldata;
    uint32_t filllen;

    uint32_t hashvalue;
    map<uint32_t,st_DataInfo>::iterator hashit; 
    st_DataInfo stDataInfo;
    int i=0;
    int ipElemNums=0,keynum=0;
    GetIpElemNumIpv6(fd,ipElemNums,keynum);
    printf("----%d-------]\n", ipElemNums);
    fwrite(&ipElemNums, sizeof(ipElemNums), 1, m_outFd);

    //uint32_t uDataAreaOffset = DAT_HEAD_SIZE + 8*256 + ipElemNums*9;
    uint32_t uDataAreaOffset = DAT_HEAD_SIZE + 8*256 + ipElemNums*sizeof(IndexInfo);
    fseek(m_outFd, uDataAreaOffset, SEEK_SET);
    uint32_t curroffset=uDataAreaOffset;

}
