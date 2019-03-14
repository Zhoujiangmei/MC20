
#include "Common.h"

#define MOSQ_MSB(A)         (uint8)((A & 0xFF00) >> 8)
#define MOSQ_LSB(A)         (uint8)(A & 0x00FF)
#define BUFFER_SIZE        512// (0x01<<20)   //512
#define PROTOCOL_NAME       "EDP"
#define PROTOCOL_VERSION    1

#define MAX_FLOAT_DPS_COUNT 1000
/*----------------------------������-----------------------------------------*/
#define ERR_UNPACK_CONNRESP_REMAIN              -1000
#define ERR_UNPACK_CONNRESP_FLAG                -1001
#define ERR_UNPACK_CONNRESP_RTN                 -1002
#define ERR_UNPACK_PUSHD_REMAIN                 -1010
#define ERR_UNPACK_PUSHD_DEVID                  -1011
#define ERR_UNPACK_PUSHD_DATA                   -1012
#define ERR_UNPACK_SAVED_REMAIN                 -1020
#define ERR_UNPACK_SAVED_TANSFLAG               -1021
#define ERR_UNPACK_SAVED_DEVID                  -1022
#define ERR_UNPACK_SAVED_DATAFLAG               -1023
#define ERR_UNPACK_SAVED_JSON                   -1024
#define ERR_UNPACK_SAVED_PARSEJSON              -1025
#define ERR_UNPACK_SAVED_BIN_DESC               -1026
#define ERR_UNPACK_SAVED_PARSEDESC              -1027
#define ERR_UNPACK_SAVED_BINLEN                 -1028
#define ERR_UNPACK_SAVED_BINDATA                -1029
#define ERR_UNPACK_PING_REMAIN                  -1030
#define ERR_UNPACK_CMDREQ                       -1031
#define ERR_UNPACK_ENCRYPT_RESP                 -1032
#define ERR_UNPACK_SAVEDATA_ACK                 -1033
#define ERR_UNPACK_SAVED_STR_WITH_TIME		    -1034
#define ERR_UNPACK_SAVED_FLOAT_WITH_TIME        -1035
#define ERR_UNPACK_UPDATE_RESP                  -1040
/*----------------------------��Ϣ����---------------------------------------*/
/* �������� */
#define CONNREQ             0x10
/* ������Ӧ */
#define CONNRESP            0x20
/* ת��(͸��)���� */
#define PUSHDATA            0x30
/* �������� */
#define UPDATEREQ           0x50
/* ������Ӧ */
#define UPDATERESP          0x60
/* �洢(ת��)���� */
#define SAVEDATA            0x80
/* �洢ȷ�� */
#define SAVEACK             0x90
/* �������� */
#define CMDREQ              0xA0
/* ������Ӧ */
#define CMDRESP             0xB0
/* �������� */
#define PINGREQ             0xC0
/* ������Ӧ */
#define PINGRESP            0xD0
/* �������� */
#define ENCRYPTREQ          0xE0
/* ������Ӧ */
#define ENCRYPTRESP         0xF0

#define UPDATE  0x50

typedef struct UpdateInfoList
{
    char* name;
    char* version;
    char* url;
    char* md5; /* 32bytes */
    struct UpdateInfoList* next;
}UpdateInfoList;

typedef enum {
    kTypeFullJson = 0x01,
    kTypeBin = 0x02,
    kTypeSimpleJsonWithoutTime = 0x03,
    kTypeSimpleJsonWithTime = 0x04,
    kTypeString = 0x05,
    kTypeStringWithTime = 0x06,
    kTypeFloatWithTime  = 0x07
}SaveDataType;

typedef struct Buffer
{
uint8*  _data;          /* buffer���� */
uint32  _write_pos;     /* bufferд��λ�� */
uint32  _read_pos;      /* buffer��ȡλ�� */
uint32  _capacity;      /* buffer���� */
}Buffer, SendBuffer, RecvBuffer, EdpPacket;

void my_assert(uint8 a);
EdpPacket* PacketConnect1(const char* devid, const char* auth_key);
EdpPacket* PacketPing(void);
Buffer* NewBuffer();
void DeleteBuffer(Buffer** buf);
int32 WriteRemainlen(Buffer* buf, uint32 len_val);
int32 WriteByte(Buffer* buf, uint8 byte);
int32 WriteBytes(Buffer* buf, const void* bytes, uint32 count);
int32 WriteStr(Buffer* buf, const char *str);
int32 WriteUint16(Buffer* buf, uint16 val);
int32 WriteUint32(Buffer* buf, uint32 val);
int32 CheckCapacity(Buffer* buf, uint32 len);
EdpPacket* PacketSavedataSimpleString(const int8* dst_devid, const int8* input);
void  apptest(void );

int32 ReadByte(EdpPacket* pkg, uint8* val);
int32 ReadBytes(EdpPacket* pkg, uint8** val, uint32 count);
int32 ReadUint16(EdpPacket* pkg, uint16* val);
int32 ReadUint32(EdpPacket* pkg, uint32* val);
int32 ReadFloat(EdpPacket* pkg, float* val);
int32 ReadStr(EdpPacket* pkg, char** val);
int32 ReadRemainlen(EdpPacket* pkg, uint32* len_val);

EdpPacket* GetEdpPacket(RecvBuffer* buf);
uint8 EdpPacketType(EdpPacket* pkg);
int32 IsPkgComplete(RecvBuffer* buf);
int32 UnpackConnectResp(EdpPacket* pkg);
EdpPacket* PacketUpdateReq(UpdateInfoList* head);
int UnpackUpdateResp(EdpPacket* pkg, UpdateInfoList** head);
void FreeUpdateInfolist(UpdateInfoList* head);