//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
//#include <assert.h>
//#include <time.h>
#include "EdpKit.h"
#include "Ql_stdlib.h"
#include "EdpDemo.h"
#include "ql_memory.h"
#include "debug.h"



void my_assert(uint8 a)
{    
      if(a == 0)   
     {        //printf("assert false\n");    
     }
}
                            

/*---------------------------------------------------------------------------*/
/* connect1 (C->S): devid + apikey */
EdpPacket* PacketConnect1(const char* devid, const char* auth_key)
{
    EdpPacket* pkg = NewBuffer();
    uint32 remainlen;
	/* msg type */
    WriteByte(pkg, CONNREQ);
	/* remain len */
	remainlen = (2+3)+1+1+2+(2+Ql_strlen(devid))+(2+Ql_strlen(auth_key));
	WriteRemainlen(pkg, remainlen);
	/* protocol desc */
	WriteStr(pkg, PROTOCOL_NAME);
	/* protocol version */
	WriteByte(pkg, PROTOCOL_VERSION);
	/* connect flag */
	WriteByte(pkg, 0x40);
	/* keep time */
      WriteUint16(pkg, 300);
	/* DEVID */
	WriteStr(pkg, devid);
	/* auth key */
	WriteStr(pkg, auth_key);
    return pkg;
}



/* ping (C->S) */
EdpPacket* PacketPing(void)
{
    EdpPacket* pkg = NULL;

    pkg = NewBuffer();
    /* msg type */
    WriteByte(pkg, PINGREQ);
    /* remain len */
    WriteRemainlen(pkg, 0);
    return pkg;
}


Buffer* NewBuffer()
{
    Buffer* buf = (Buffer*)Ql_MEM_Alloc(sizeof(Buffer));
    if(buf == NULL)
    {
         APP_DEBUG("<-- memary buf alloc err -->\r\n");
    }
    buf->_data = (uint8*)Ql_MEM_Alloc(sizeof(uint8) * BUFFER_SIZE);
    if(buf->_data == NULL)
   {
	APP_DEBUG("<-- memary data alloc err -->\r\n");
   }
   APP_DEBUG("<-- memary  alloc sus-->\r\n");
    buf->_write_pos = 0;
    buf->_read_pos = 0;
    buf->_capacity = BUFFER_SIZE;
    return buf;
}


void DeleteBuffer(Buffer** buf)
{
    uint8* pdata = (*buf)->_data;
    Ql_MEM_Free(pdata);
    Ql_MEM_Free(*buf);
    *buf = 0;
}


/*---------------------------------------------------------------------------*/
int32 ReadByte(EdpPacket* pkg, uint8* val)
{
    if (pkg->_read_pos+1 > pkg->_write_pos) 
        return -1;
    *val = pkg->_data[pkg->_read_pos];
    pkg->_read_pos += 1;
    return 0;
}
int32 ReadBytes(EdpPacket* pkg, uint8** val, uint32 count)
{
    if (pkg->_read_pos+count > pkg->_write_pos) 
        return -1;
    *val = (uint8*)Ql_MEM_Alloc(sizeof(uint8) * count);
    Ql_memcpy(*val, pkg->_data + pkg->_read_pos, count);
    pkg->_read_pos += count;
    return 0;
}
int32 ReadUint16(EdpPacket* pkg, uint16* val)
{
    uint8 msb, lsb;
    if (pkg->_read_pos+2 > pkg->_write_pos) 
        return -1;
    msb = pkg->_data[pkg->_read_pos];
    pkg->_read_pos++;
    lsb = pkg->_data[pkg->_read_pos];
    pkg->_read_pos++;
    *val = (msb<<8) + lsb;
    return 0;
}
int32 ReadUint32(EdpPacket* pkg, uint32* val)
{
    int32 i = 0;
    uint32 tmpval = 0;
    if (pkg->_read_pos+4 > pkg->_write_pos) 
        return -1;
    while (i++ < 4) {
        tmpval = (tmpval << 8) | (pkg->_data[pkg->_read_pos]);
        pkg->_read_pos++;
    }
    *val = tmpval;
    return 0;
}

int32 ReadFloat(EdpPacket* pkg, float* val)
{
    *val = 0;
    if (pkg->_read_pos + 4 > pkg->_write_pos)
        return -1;

    Ql_memcpy(val, pkg->_data + pkg->_read_pos, sizeof(float));
    pkg->_read_pos += 4;
    return 0;
}

int32 ReadStr(EdpPacket* pkg, char** val)
{
    uint16 len = 0;
    int rc = 0;
    /* read str len */
    rc = ReadUint16(pkg, &len);
    if (rc) 
        return rc;
    if (pkg->_read_pos+len > pkg->_write_pos) 
        return -1;
    /* copy str val */
    *val = (char*)Ql_MEM_Alloc(sizeof(char) * (len + 1));
    Ql_memset(*val, 0, len+1);
    Ql_strncpy(*val, (const char *)(pkg->_data + pkg->_read_pos), len);
    pkg->_read_pos += len;
    return 0;
}



int32 ReadRemainlen(EdpPacket* pkg, uint32* len_val)
{
    uint32 multiplier = 1;
    uint32 len_len = 0;
    uint8 onebyte = 0;
    int32 rc;
    *len_val = 0;
    do {
        rc = ReadByte(pkg, &onebyte);
        if (rc) 
            return rc;

        *len_val += (onebyte & 0x7f) * multiplier;
        multiplier *= 0x80;

        len_len++;
        if (len_len > 4) {
            return -1;/*len of len more than 4;*/
        }
    } while((onebyte & 0x80) != 0);
    return 0;
}




int32 WriteRemainlen(Buffer* buf, uint32 len_val)
{
    uint32 remaining_length = len_val;
    int32 remaining_count = 0;
    uint8 byte = 0;

    my_assert(buf->_read_pos == 0);

    do {
        byte = remaining_length % 128;
        remaining_length = remaining_length / 128;
        /* If there are more digits to encode, set the top bit of this digit */
        if (remaining_length > 0) {
            byte = byte | 0x80;
        }
        buf->_data[buf->_write_pos++] = byte;
        remaining_count++;
    }while(remaining_length > 0 && remaining_count < 5);
    my_assert(remaining_count != 5);
    return 0;
}


int32 WriteByte(Buffer* buf, uint8 byte)
{

    my_assert(buf->_read_pos == 0);
    if (CheckCapacity(buf, 1))
    {
        return -1;
    }
    buf->_data[buf->_write_pos] = byte;
    buf->_write_pos++;
    return 0;
}

int32 WriteBytes(Buffer* buf, const void* bytes, uint32 count)
{
    my_assert(buf->_read_pos == 0);
    if (CheckCapacity(buf, count))
        return -1;
    Ql_memcpy(buf->_data + buf->_write_pos, bytes, count);
    buf->_write_pos += count;
    return 0;
}

int32 WriteStr(Buffer* buf, const char *str)
{
    uint16 length = 0;
    my_assert(buf->_read_pos == 0);
    length = Ql_strlen(str);
    return WriteUint16(buf, length) 
        || WriteBytes(buf, str, length);
}



int32 WriteUint16(Buffer* buf, uint16 val)
{
    my_assert(buf->_read_pos == 0);
    return  WriteByte(buf, MOSQ_MSB(val))  || WriteByte(buf, MOSQ_LSB(val)) ;
}

int32 WriteUint32(Buffer* buf, uint32 val)
{
    my_assert(buf->_read_pos == 0);
    return WriteByte(buf, (val >> 24) & 0x00FF)
           || WriteByte(buf, (val >> 16) & 0x00FF)
           || WriteByte(buf, (val >> 8) & 0x00FF)
           || WriteByte(buf, (val) & 0x00FF);
}


int32 CheckCapacity(Buffer* buf, uint32 len)
{
    uint32 cap_len = buf->_capacity;
    int32 flag = 0;
    uint8* pdata = NULL;
    APP_DEBUG("buf->_capacity = %d, buf->_write_pos = %d, len = %d\r\n", buf->_capacity, buf->_write_pos,len);
    while (cap_len - buf->_write_pos < len) /* remain len < len */
    {
        cap_len = cap_len << 1;
        if (++flag > 32)
            break;  /* overflow */
    }
    if (flag > 32)
    {
        APP_DEBUG("<-- leave CheckCapacity err -->\r\n");
        return -1;
    }
    if (cap_len > buf->_capacity)
    {
        pdata = (uint8*)Ql_MEM_Alloc(sizeof(uint8) * cap_len);
        Ql_memcpy(pdata, buf->_data, buf->_write_pos);
        Ql_MEM_Free(buf->_data);
        buf->_data = pdata;
        buf->_capacity = cap_len;
    }
    return 0;
}


EdpPacket* PacketSavedataSimpleString(const int8* dst_devid, const int8* input)
{
    EdpPacket* pkg = NULL;
    uint32 remainlen = 0;
    uint32 input_len = 0;

    pkg = NewBuffer();
    input_len = Ql_strlen((const char *)input);
    /* msg type */
    WriteByte(pkg, SAVEDATA);
    if (dst_devid)
    {
        /* remain len */
        remainlen = 1 + (2 + Ql_strlen((const char *)dst_devid)) + 1 + (2 + input_len);
        WriteRemainlen(pkg, remainlen);
        /* translate address flag */
        WriteByte(pkg, 0x80);
        /* dst devid */
        WriteStr(pkg, dst_devid);
    }
    else
    {
        /* remain len */
        remainlen = 1 + 1 + (2 + input_len);
        WriteRemainlen(pkg, remainlen);
        /* translate address flag */
        WriteByte(pkg, 0x00);
    }
    /* json flag */
    WriteByte(pkg, kTypeString);
    /* json */
    WriteStr(pkg, input);

    return pkg;
}



void apptest(void )
{
    APP_DEBUG("<-- test ok -->\r\n");
}


/* recv stream to a edp packet (S->C) */
EdpPacket* GetEdpPacket(RecvBuffer* buf)
{
    EdpPacket* pkg = NULL;
    int32 flag = 0;
 
    my_assert(buf->_read_pos == 0);
    flag = IsPkgComplete(buf);  
    if (flag <= 0)
 return pkg;
    pkg = NewBuffer();
    WriteBytes(pkg, buf->_data, flag);

    /* shrink buffer */
    APP_DEBUG("buf->_data = %s, buf->_write_pos = %d, flag = %d\r\n ", buf->_data, buf->_write_pos, flag);
    Ql_memmove(buf->_data, buf->_data + flag, buf->_write_pos - flag);
    buf->_write_pos -= flag;
    return pkg;
}


/* get edp packet type, client should use this type to invoke Unpack??? function */
uint8 EdpPacketType(EdpPacket* pkg)
{
    uint8 mtype = 0x00;
    ReadByte(pkg, &mtype);
    return mtype;
}


/* is the recv buffer has a complete edp packet? */
int32 IsPkgComplete(RecvBuffer* buf)
{
    uint8* data = NULL;
    uint32 data_len = 0;
    uint32 multiplier = 1;
    uint32 len_val = 0;
    uint32 len_len = 1;
    uint8* pdigit = NULL;
    uint32 pkg_total_len = 0;
    
    data = buf->_data;
    data_len = buf->_write_pos;

    if (data_len <= 1) {
	return 0;   /* continue receive */
    }
    /* recevie remaining len */
    pdigit = data;

    do {
	if (len_len > 4) {
	    return -1;  /* protocol error; */
	}
	if (len_len > data_len - 1) {
	    return 0;   /* continue receive */
	}
	len_len++;
	pdigit++;
	len_val += ((*pdigit) & 0x7f) * multiplier;
	multiplier *= 0x80;
    }while(((*pdigit) & 0x80) != 0);

    pkg_total_len = len_len + len_val;
    /* receive payload */
    if (pkg_total_len <= (uint32)data_len){
#ifdef _DEBUG
	//printf("a complete packet len:%d\n", pkg_total_len);
#endif
	return pkg_total_len;   /* all data for this pkg is read */
    }else{
	return 0;   /* continue receive */
    }
}


int32 UnpackConnectResp(EdpPacket* pkg)
{
    uint8 flag, rtn;
    uint32 remainlen;
    if (ReadRemainlen(pkg, &remainlen))
        return ERR_UNPACK_CONNRESP_REMAIN;
    if (ReadByte(pkg, &flag))
        return ERR_UNPACK_CONNRESP_FLAG;
    if (ReadByte(pkg, &rtn))
        return ERR_UNPACK_CONNRESP_RTN;
    my_assert(pkg->_read_pos == pkg->_write_pos);
    return (int32)rtn;
}


EdpPacket* PacketUpdateReq(UpdateInfoList* head)
{
    EdpPacket* pkg = NULL;
    uint32 len = 0;
    UpdateInfoList* node = head;

    pkg = NewBuffer();
    WriteByte(pkg, UPDATEREQ);

    while (node){
        if (!node->name || !Ql_strlen(node->name)
            || !node->version || !Ql_strlen(node->name)) {
            DeleteBuffer(&pkg);
            return NULL;
        }
        len += Ql_strlen(node->name) + Ql_strlen(node->version) + 4;
        node = node->next;
    }

    WriteRemainlen(pkg, len);
    node = head;
    while (node){
        WriteStr(pkg, node->name);
        WriteStr(pkg, node->version);
        node = node->next;
    }

    return pkg;
}


void FreeUpdateInfolist(UpdateInfoList* head)
{
    UpdateInfoList* next = NULL;
    while (head){
        next = head->next ?  head->next : NULL;

        if (head->name){
            Ql_MEM_Free(head->name);
        }
        if (head->version){
            Ql_MEM_Free(head->version);
        }
        if (head->url){
            Ql_MEM_Free(head->version);
        }
        if (head->md5){
            Ql_MEM_Free(head->md5);
        }
        Ql_MEM_Free(head);
        head = next;
    }
}

int UnpackUpdateResp(EdpPacket* pkg, UpdateInfoList** head)
{
    uint32 remainlen = 0;
    uint32 cur_pos = 0;
    const char* p = NULL;
    UpdateInfoList* node = NULL;

    if (ReadRemainlen(pkg, &remainlen)){
        return ERR_UNPACK_UPDATE_RESP;
    }

    while (pkg->_read_pos != pkg->_write_pos){
        if (node){
            node->next = (UpdateInfoList*)Ql_MEM_Alloc(sizeof(UpdateInfoList));
            node = node->next;
        }
        else{
            *head = (UpdateInfoList*)Ql_MEM_Alloc(sizeof(UpdateInfoList));
            node = *head;
        }
        node->next = NULL;

        if (ReadStr(pkg, &node->name) || ReadStr(pkg, &node->version)
            || ReadStr(pkg, &node->url) || ReadBytes(pkg, (uint8**)&node->md5, 32)){
            return -1;
        }
    }
}