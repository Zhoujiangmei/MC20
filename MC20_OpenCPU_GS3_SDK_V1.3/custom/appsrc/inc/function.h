
typedef struct firmwareInfo
{
    char name[20];
    char version[20];

}firmwareInfo;

extern firmwareInfo firmwareInfomation;
void parameterConfigure(void );
void parameterRead(void );
void parameterWrite(void );

void CreateDataFile(void );