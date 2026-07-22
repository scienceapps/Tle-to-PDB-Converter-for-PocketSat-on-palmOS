//
// Convert a text TLE ephemeris file into a pocketSat input database
//
// usage: tle2pdb tleFile pdbFile dbName
//
// NOTE: make sure that your compiler byte-aligns when you build this.
// Actually, word-alignment may be OK. Since there isn't a time/size constraint
// I just went with bytes.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

//
// palm PDB file structure
//

#define UWord unsigned short
#define ULong unsigned long
#define UByte unsigned char

#define DBNAME_LEN 32

// DB attributes
#define DB_READONLY 0x0002
#define DB_OVERWRITE 0x0010

typedef struct _pdbhead
{
    char  name[DBNAME_LEN];  // needs null terminator
    UWord attr;
    UWord version;
    ULong cdate;
    ULong mdate;
    ULong bdate;
    ULong modNum;
    ULong appInfoOffset;
    ULong sortInfoOffset;
    char  dbType[4];
    char  creatorID[4];
    ULong idSeed;
    ULong nextRecListID;
    UWord numRec;
} PDBHeader;

typedef struct _reclistentry
{
    ULong recOffset;
    UByte attrAndID[4];
} RecordListEntry;

//
// ID and type
//
#define CREATOR_ID "pSat"
#define DB_TYPE    "iTLE"

#define MAX_RECS 3000 // that would be 1000 satellites

// I'm pretty sure these swap functions work automagically.
// I've used this method before and it worked, but I don't have
// access to a Motorola-based platform to test on.

UWord order = 'MI';
UWord MSBWord(UWord in)
{
    UWord res;
    char* pC = (char*)&order;

    if (*pC == 'M')
        return in;

    res = (in >> 8) + ((in & 0xff)<<8);

    return res;
}

ULong MSBLong(ULong in)
{
    ULong res;
    char* pC = (char*)&order;

    if (*pC == 'M')
        return in;

    res = (in >> 24)
        + ((in & 0x00ff0000) >> 8)
        + ((in & 0x0000ff00) << 8)
        + ((in & 0x000000ff) << 24);

    return res;
}

// Should be secs since Jan 1, 1904 
//(or so the docs claim. Actually, from looking at PDB headers,
// it seems to be from 1/1/1970)
ULong PalmSecsNow(void)
{
    ULong t = time(&t); // since Jan 1, 1970

    return t;
}

void main(int argc, char** argv)
{
    FILE* inFile;
    FILE* outFile;
    char dbName[DBNAME_LEN];
    char* inRecs[MAX_RECS];
    char inBuff[132];
    UWord recCount;
    PDBHeader head;
    RecordListEntry* recList;
    ULong dataLoc;
    int i;

    if (argc < 3)
    {
        puts("usage: tle2pdb inputFile outputFile databaseName");
        puts("   ie: tle2pdb sats.tle  sats.pdb   NewSatellites");
        return;
    }

    if ((inFile = fopen(argv[1], "rt")) == NULL)
    {
        perror("Can't open input file");
        return;
    }

    if ((outFile = fopen(argv[2], "wb")) == NULL)
    {
        perror("Can't open output file");
        fclose(inFile);
        return;
    }

    strncpy(dbName,argv[3], DBNAME_LEN);
    dbName[DBNAME_LEN-1] = 0;

    // pretty small files, so just read it all in while  counting recs
    recCount = 0;
    while( fgets(inBuff, 132, inFile) != NULL)
    {
        int n = strlen(inBuff);

        inRecs[recCount] = malloc(n+1);
        strcpy(inRecs[recCount],inBuff);
        recCount++;
    
        if (recCount == MAX_RECS)
        {
            puts("Too many input records. Data truncated.");
            break;
        }
    
    }

    fclose(inFile);

    // initialize the header
    strcpy(head.name, dbName);
    //head.attr = MSBWord( DB_OVERWRITE);
    head.attr = 0;
    head.version = MSBWord(1);
    head.cdate = MSBLong(PalmSecsNow());
    head.mdate = head.cdate;
    head.bdate = 0;
    head.modNum = 0;
    head.appInfoOffset = 0;
    head.sortInfoOffset = 0;
    memcpy(head.dbType,DB_TYPE,4);
    memcpy(head.creatorID,CREATOR_ID,4);
    head.idSeed = 0;
    head.nextRecListID = 0;
    head.numRec = MSBWord(recCount);

    // create and fill the record list block
    recList = (RecordListEntry*)malloc( recCount*sizeof(RecordListEntry));

    dataLoc = sizeof(PDBHeader) + recCount * sizeof(RecordListEntry);

    for(i=0;i<recCount;i++)
    {
        memset(&recList[i], 0, sizeof(RecordListEntry));
        recList[i].recOffset = MSBLong(dataLoc);
        dataLoc += strlen(inRecs[i]);
    }

    // write it all out
    fwrite(&head, sizeof(PDBHeader),1,outFile);
    fwrite(recList, sizeof(RecordListEntry),recCount,outFile);
    free(recList);

    for(i=0;i<recCount;i++)
    {
        fwrite(inRecs[i],strlen(inRecs[i]),1,outFile);
        free(inRecs[i]);
    }

    fclose(outFile);


}