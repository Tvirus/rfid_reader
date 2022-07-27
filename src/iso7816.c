#include "iso7816.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>




#define CMD_VERIFY_CLA         0x00
#define CMD_VERIFY_INS         0x20
#define CMD_EXT_AUTHN_CLA      0x00
#define CMD_EXT_AUTHN_INS      0x82
#define CMD_GET_CHALLENGE_CLA  0x00
#define CMD_GET_CHALLENGE_INS  0x84
#define CMD_INT_AUTHN_CLA      0x00
#define CMD_INT_AUTHN_INS      0x88
#define CMD_SELECT_CLA         0x00
#define CMD_SELECT_INS         0xA4
#define CMD_READ_BINARY_CLA    0x00
#define CMD_READ_BINARY_INS    0xB0
#define CMD_READ_RECORD_CLA    0x00
#define CMD_READ_RECORD_INS    0xB2
#define CMD_GET_RESPONSE_CLA   0x00
#define CMD_GET_RESPONSE_INS   0xC0
#define CMD_WRITE_BINARY_CLA   0x00
#define CMD_WRITE_BINARY_INS   0xD6
#define CMD_WRITE_RECORD_CLA   0x00
#define CMD_WRITE_RECORD_INS   0xD2
#define CMD_UPDATE_BINARY_CLA  0x00
#define CMD_UPDATE_BINARY_INS  0xD6

#define CMD_ERASE_DF_CLA       0x80
#define CMD_ERASE_DF_INS       0x0E
#define CMD_WRITE_KEY_CLA      0x80
#define CMD_WRITE_KEY_INS      0xD4
#define CMD_CREATE_FILE_CLA    0x80
#define CMD_CREATE_FILE_INS    0xE0


#define OK_SW1  0x90
#define OK_SW2  0x00




#define DEBUG(fmt, arg...)  do{if(iso7816_debug)printf("--ISO7816-- " fmt "\n", ##arg);}while(0)
#define ERROR(fmt, arg...)  printf("--ISO7816-- " fmt "\n", ##arg)


typedef struct
{
    u8 cla;
    u8 ins;
    u8 p1;
    u8 p2;
    u8 data[0];
}cmd_apdu_t;



int iso7816_debug = 1;





char* iso7816_strerror(const u8 *sw)
{
    if (NULL == sw)
        return "";

    if (0x62 == sw[0])
    {
        if (0x02 <= sw[1] && 0x80 >= sw[1]) return "Triggering by the card";
        else if (0x81 == sw[1]) return "Part of returned data may be corrupted";
        else if (0x82 == sw[1]) return "End of file or record reached before reading Ne bytes";
        else if (0x83 == sw[1]) return "Selected file deactivated";
        else if (0x84 == sw[1]) return "File control information not formatted";
        else if (0x85 == sw[1]) return "Selected file in termination state";
        else if (0x86 == sw[1]) return "No input data available from a sensor on the card";
    }
    else if (0x63 == sw[0])
    {
        if (0x81 == sw[1]) return "File filled up by the last write";
        else if (0xC0 == (sw[1] & 0xF0)) return "Counter from 0 to 15 encoded by 'X'(SW2&0xF)";
    }
    else if (0x64 == sw[0])
    {
        if (0x01 == sw[1]) return "Immediate response required by the card";
        else if (0x02 <= sw[1] && 0x80 >= sw[1]) return "Triggering by the card";
    }
    else if (0x65 == sw[0])
    {
        if (0x81 == sw[1]) return "Memory failure";
    }
    else if (0x68 == sw[0])
    {
        if      (0x81 == sw[1]) return "Logical channel not supported";
        else if (0x82 == sw[1]) return "Secure messaging not supported";
        else if (0x83 == sw[1]) return "Last command of the chain expected";
        else if (0x84 == sw[1]) return "Command chaining not supported";
    }
    else if (0x69 == sw[0])
    {
        if      (0x81 == sw[1]) return "Command incompatible with file structure";
        else if (0x82 == sw[1]) return "Security status not satisfied";
        else if (0x83 == sw[1]) return "Authentication method blocked";
        else if (0x84 == sw[1]) return "Reference data not usable";
        else if (0x85 == sw[1]) return "Conditions of use not satisfied";
        else if (0x86 == sw[1]) return "Command not allowed (no current EF)";
        else if (0x87 == sw[1]) return "Expected secure messaging data objects missing";
        else if (0x88 == sw[1]) return "Incorrect secure messaging data objects";
    }
    else if (0x6A == sw[0])
    {
        if      (0x80 == sw[1]) return "Incorrect parameters in the command data field";
        else if (0x81 == sw[1]) return "Function not supported";
        else if (0x82 == sw[1]) return "File or application not found";
        else if (0x83 == sw[1]) return "Record not found";
        else if (0x84 == sw[1]) return "Not enough memory space in the file";
        else if (0x85 == sw[1]) return "Nc inconsistent with TLV structure";
        else if (0x86 == sw[1]) return "Incorrect parameters P1-P2";
        else if (0x87 == sw[1]) return "Nc inconsistent with parameters P1-P2";
        else if (0x88 == sw[1]) return "Referenced data or reference data not found";
        else if (0x89 == sw[1]) return "File already exists";
        else if (0x8A == sw[1]) return "DF name already exists";
    }

    return "Unknown";
}

//level: 用于控制缩进
void print_simple_tlv(u8 *tlv, int len, u8 level)
{
    int i, j;

    if (2 > len)
    {
        printf("length(%d) err !\n", len);
        return;
    }
    if (34 < level)
        return;

    while (0 < len)
    {
        for (i = 0; i < level; i++)
            printf("    ");

        if (tlv[1] > (len - 2))
        {
            printf("length(%u) err:", len);
            for (j = 0; j < len; j++)
                printf(" %02x", tlv[j]);
            printf("\n");
            return;
        }
        if (0x62 == tlv[0])
        {
            printf("62: FCP(File control parameters)\n");
            print_simple_tlv(tlv + 2, tlv[1], level + 1);
        }
        else if (0x64 == tlv[0])
        {
            printf("64: FMD(File management data)\n");
            print_simple_tlv(tlv + 2, tlv[1], level + 1);
        }
        else if (0x6F == tlv[0])
        {
            printf("6F: FCI(File control information)\n");
            print_simple_tlv(tlv + 2, tlv[1], level + 1);
        }
        else if (0x80 == tlv[0])
        {
            printf("80: Number of data bytes in the file:");
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else if (0x81 == tlv[0])
        {
            printf("81: Number of data bytes in the file:");
            if (2 != tlv[1])
                printf("(length(%u) is not 2)", tlv[1]);
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else if (0x82 == tlv[0])
        {
            printf("82: File descriptor:");
            if (6 < tlv[1])
                printf("(length(%u) > 6)", tlv[1]);
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else if (0x83 == tlv[0])
        {
            printf("83: File identifier:");
            if (2 != tlv[1])
                printf("(length(%u) is not 2)", tlv[1]);
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else if (0x84 == tlv[0])
        {
            printf("84: DF name:");
            if (16 < tlv[1])
                printf("(length(%u) > 16)", tlv[1]);
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("(");
            for (j = 0; j < tlv[1]; j++)
                printf("%c", (isprint(tlv[2 + j])? tlv[2 + j]:'.'));
            printf(")\n");
        }
        else if (0x85 == tlv[0])
        {
            printf("85: Proprietary information:");
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else if (0x86 == tlv[0])
        {
            printf("86: Security attribute proprietary:");
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else if (0x87 == tlv[0])
        {
            printf("87: Identifier of an EF:");
            if (2 != tlv[1])
                printf("(length(%u) is not 2)", tlv[1]);
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else if (0x88 == tlv[0])
        {
            printf("88: Short EF identifier:");
            if (1 < tlv[1])
                printf("(length(%u) > 1)", tlv[1]);
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else if (0x8A == tlv[0])
        {
            printf("8A: Life cycle status byte:");
            if (1 != tlv[1])
                printf("(length(%u) is not 1)", tlv[1]);
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else if (0x8B == tlv[0])
        {
            printf("8B: Security attribute expanded:");
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else if (0x8C == tlv[0])
        {
            printf("8C: Security attribute compact:");
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else if (0x8D == tlv[0])
        {
            printf("8D: Identifier of an EF containing security environment templates:");
            if (2 != tlv[1])
                printf("(length(%u) is not 2)", tlv[1]);
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else if (0x8E == tlv[0])
        {
            printf("8E: Identifier of an EF containing security environment templates:");
            if (1 != tlv[1])
                printf("(length(%u) is not 1)", tlv[1]);
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else if (0xA0 == tlv[0])
        {
            printf("A0: Security attribute template for data objects:");
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else if (0xA1 == tlv[0])
        {
            printf("A1: Security attribute template in proprietary format:");
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else if (0xA2 == tlv[0])
        {
            printf("A2: Template of data objects:");
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else if (0xA5 == tlv[0])
        {
            printf("A5: Proprietary information:\n");
            //print_ber_tlv(tlv + 2, tlv[1], level + 1);
            print_simple_tlv(tlv + 2, tlv[1], level + 1);
        }
        else if (0xAB == tlv[0])
        {
            printf("AB: Security attribute template in expanded format:");
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else if (0xAC == tlv[0])
        {
            printf("AC: Cryptographic mechanism identifier template:");
            for (j = 0; j < tlv[1]; j++)
                printf(" %02x", tlv[2 + j]);
            printf("\n");
        }
        else
        {
            printf("Unknown:");
            for (j = 0; j < 2 + tlv[1]; j++)
                printf(" %02x", tlv[j]);
            printf("\n");
        }
        len -= 2 + tlv[1];
        tlv += 2 + tlv[1];
    }
    return;
}

//value: 指定tag的value指针
int find_simple_tlv_tag(u8 *tlv, u8 len, u8 tag, u8 **value)
{
    if ((NULL == tlv) || (2 > len) || (NULL == value))
        return -1;
    return -1;
}

int iso7816_select_id(card_info_t *info, u16 id, u8 *buf, int buf_len)
{
    u8 send[sizeof(cmd_apdu_t) + 3];
    cmd_apdu_t *cmd = (cmd_apdu_t *)send;
    u8 recv[100];
    int len = 0;
    int i;


    if (NULL == info)
        return -1;

    cmd->cla = CMD_SELECT_CLA;
    cmd->ins = CMD_SELECT_INS;
    cmd->p1  = 0;
    cmd->p2  = 0;
    cmd->data[0] = 2;
    cmd->data[1] = (id >> 8) & 0xFF;
    cmd->data[2] = id & 0xFF;
    DEBUG("Select %04x, cmd(7): %02x %02x %02x %02x %02x  %02x %02x",
           id, send[0], send[1], send[2], send[3], send[4], send[5], send[6]);
    len = typeab_send(info, send, sizeof(send), recv, sizeof(recv));
    if (2 > len)
    {
        ERROR("%s: typeab_send() failed(%d) !", __func__, len);
        return -1;
    }
    if (sizeof(recv) < len)
    {
        ERROR("%s: response too long(%d) !", __func__, len);
        return -1;
    }
    if ((OK_SW1 != recv[len - 2]) || (OK_SW2 != recv[len - 1]))
    {
        ERROR("select id %04x failed, %02X %02X: %s",
               id, recv[len - 2], recv[len - 1], iso7816_strerror(&recv[len - 2]));
        return -1;
    }
    if (iso7816_debug)
    {
        printf("--ISO7816-- Select OK(%d): ", len);
        for (i = 0; i < len; i++)
        {
            printf("%02x ", recv[i]);
        }
        printf("\n");
    }

    if ((NULL == buf) || (0 == buf_len))
    {
        return 0;
    }

    len -= 2;
    if (iso7816_debug && len)
    {
        printf("================ TLV ================\n");
        print_simple_tlv(recv, len, 0);
        printf("=====================================\n");
    }
    memcpy(buf, recv, (buf_len < len)? buf_len : len);
    return len;
}


int iso7816_get_challenge(card_info_t *info, u8 challenge_len, u8 *challenge)
{
    u8 send[sizeof(cmd_apdu_t) + 1];
    cmd_apdu_t *cmd = (cmd_apdu_t *)send;
    u8 recv[8 + 2];
    int len = 0;
    int i;


    if ((NULL == info) || (NULL == challenge) || (8 < challenge_len) || (0 == challenge_len))
        return -1;

    cmd->cla = CMD_GET_CHALLENGE_CLA;
    cmd->ins = CMD_GET_CHALLENGE_INS;
    cmd->p1  = 0;
    cmd->p2  = 0;
    cmd->data[0] = challenge_len;
    DEBUG("Get challenge %u, cmd(5): %02x %02x %02x %02x %02x",
           challenge_len, send[0], send[1], send[2], send[3], send[4]);
    len = typeab_send(info, send, sizeof(send), recv, sizeof(recv));
    if ((challenge_len + 2) != len)
    {
        ERROR("get challenge failed(%d) !", len);
        return -1;
    }
    if ((OK_SW1 != recv[len - 2]) || (OK_SW2 != recv[len - 1]))
    {
        ERROR("get challenge failed, %02X %02X: %s", \
               recv[len - 2], recv[len - 1], iso7816_strerror(&recv[len - 2]));
        return -1;
    }
    if (iso7816_debug)
    {
        printf("--ISO7816-- Get challenge OK(%u): ", challenge_len + 2);
        for (i = 0; i < (challenge_len + 2); i++)
        {
            printf("%02x ", recv[i]);
        }
        printf("\n");
    }

    memcpy(challenge, recv, challenge_len);
    return challenge_len;
}


int iso7816_exter_authn(card_info_t *info, u8 key_id, const u8 *data)
{
    u8 send[sizeof(cmd_apdu_t) + 1 + 8];
    cmd_apdu_t *cmd = (cmd_apdu_t *)send;
    u8 recv[2];
    int len = 0;


    if ((NULL == info) || (NULL == data))
        return -1;

    cmd->cla = CMD_EXT_AUTHN_CLA;
    cmd->ins = CMD_EXT_AUTHN_INS;
    cmd->p1  = 0;
    cmd->p2  = key_id;
    cmd->data[0] = 8;
    memcpy(&(cmd->data[1]), data, 8);
    DEBUG("External authenticate, key:%u, cmd(13): %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x", \
           key_id, send[0], send[1], send[2], send[3], send[4], send[5], send[6], \
                   send[7], send[8], send[9], send[10], send[11], send[12]);
    len = typeab_send(info, send, sizeof(send), recv, sizeof(recv));
    if (2 != len)
    {
        ERROR("external authenticate failed(%d) !", len);
        return -1;
    }
    if ((OK_SW1 != recv[0]) || (OK_SW2 != recv[1]))
    {
        ERROR("external authenticate failed, %02X %02X: %s", recv[0], recv[1], iso7816_strerror(recv));
        return -1;
    }

    DEBUG("External authenticate OK");
    return 0;
}


int iso7816_inter_authn(card_info_t *info, u8 key_id, u8 algorithm, const u8 *data, u8 *result)
{
    u8 send[sizeof(cmd_apdu_t) + 1 + 8];
    cmd_apdu_t *cmd = (cmd_apdu_t *)send;
    u8 recv[8 + 2];
    int len = 0;


    if ((NULL == info) || (NULL == data) || (NULL == result))
        return -1;

    cmd->cla = CMD_INT_AUTHN_CLA;
    cmd->ins = CMD_INT_AUTHN_INS;
    cmd->p1  = algorithm;
    cmd->p2  = key_id;
    cmd->data[0] = 8;
    memcpy(&(cmd->data[1]), data, 8);
    DEBUG("Internal authenticate, key:%u, cmd(13): %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x", \
           key_id, send[0], send[1], send[2], send[3], send[4], send[5], send[6], \
                   send[7], send[8], send[9], send[10], send[11], send[12]);
    len = typeab_send(info, send, sizeof(send), recv, sizeof(recv));
    if (2 > len)
    {
        ERROR("internal authenticate failed(%d) !", len);
        return -1;
    }
    if ((10 != len) || (OK_SW1 != recv[len - 2]) || (OK_SW2 != recv[len - 1]))
    {
        ERROR("external authenticate failed(%d), %02X %02X: %s", \
               len, recv[len - 2], recv[len - 1], iso7816_strerror(&recv[len - 2]));
        return -1;
    }

    DEBUG("Internal authenticate OK: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", \
           recv[0], recv[1], recv[2], recv[3], recv[4], recv[5], recv[6], recv[7], recv[8], recv[9]);
    memcpy(result, recv, 8);
    return 8;
}


int iso7816_read_binary(card_info_t *info, u16 offset, u8 len, u8 *buf)
{
    u8 send[sizeof(cmd_apdu_t) + 1];
    cmd_apdu_t *cmd = (cmd_apdu_t *)send;
    u8 recv[255];
    int _len = 0;
    int i;


    if ((NULL == info) || (NULL == buf) || (0 == len))
        return -1;

    cmd->cla = CMD_READ_BINARY_CLA;
    cmd->ins = CMD_READ_BINARY_INS;
    cmd->p1  = (offset >> 8) & 0xFF;
    cmd->p2  = offset & 0xFF;
    cmd->data[0] = len;
    DEBUG("Read binary, offset:%u len:%u, cmd(5): %02x %02x %02x %02x %02x", \
           offset, len, send[0], send[1], send[2], send[3], send[4]);
    _len = typeab_send(info, send, sizeof(send), recv, sizeof(recv));
    if (2 > _len)
    {
        ERROR("read binary failed(%d) !", _len);
        return -1;
    }
    if ((2 == _len) || (OK_SW1 != recv[_len - 2]) || (OK_SW2 != recv[_len - 1]))
    {
        ERROR("read binary failed(%d), %02X %02X: %s", \
               _len, recv[_len - 2], recv[_len - 1], iso7816_strerror(&recv[_len - 2]));
        return -1;
    }

    if (iso7816_debug)
    {
        printf("--ISO7816-- Read binary OK(%d):", _len);
        for (i = 0; i < _len; i++)
            printf(" %02x", recv[i]);
        printf("\n");
    }
    memcpy(buf, recv, _len - 2);
    return _len - 2;
}


int iso7816_write_binary(card_info_t *info, u16 offset, u8 len, const u8 *buf)
{
    u8 send[64];
    cmd_apdu_t *cmd = (cmd_apdu_t *)send;
    u8 recv[2];
    int _len = 0;
    int i;


    if ((NULL == info) || (NULL == buf) || (0 == len))
        return -1;
    if (sizeof(send) < (sizeof(cmd_apdu_t) + 1 + len))
        return -1;

    cmd->cla = CMD_WRITE_BINARY_CLA;
    cmd->ins = CMD_WRITE_BINARY_INS;
    cmd->p1  = (offset >> 8) & 0xFF;
    cmd->p2  = offset & 0xFF;
    cmd->data[0] = len;
    memcpy(&cmd->data[1], buf, len);
    if (iso7816_debug)
    {
        printf("--ISO7816-- Write binary, offset:%u len:%u, cmd(%u): %02x %02x %02x %02x %02x ", \
                offset, len, sizeof(cmd_apdu_t) + 1 + len, send[0], send[1], send[2], send[3], send[4]);
        for (i = 0; i < len; i++)
            printf(" %02x", buf[i]);
        printf("\n");
    }
    _len = typeab_send(info, send, sizeof(cmd_apdu_t) + 1 + len, recv, sizeof(recv));
    if (2 != _len)
    {
        ERROR("write binary failed(%d) !", _len);
        return -1;
    }
    if ((OK_SW1 != recv[0]) || (OK_SW2 != recv[1]))
    {
        ERROR("write binary failed(%d), %02X %02X: %s", \
               _len, recv[0], recv[1], iso7816_strerror(recv));
        return -1;
    }

    DEBUG("Write binary OK");
    return 0;
}




//**************************//
//以下是复旦微fmcos相关函数
//**************************//
int fmcos_erase(card_info_t *info)
{
    u8 send[sizeof(cmd_apdu_t) + 1];
    cmd_apdu_t *cmd = (cmd_apdu_t *)send;
    u8 recv[2];
    int len = 0;


    cmd->cla = 0x80;
    cmd->ins = 0x0E;
    cmd->p1  = 0;
    cmd->p2  = 0;
    cmd->data[0] = 0;
    DEBUG("Erase DF");
    len = typeab_send(info, send, sizeof(send), recv, sizeof(recv));
    if (2 != len)
    {
        ERROR("erase df failed(%d) !", len);
        return -1;
    }
    if ((OK_SW1 != recv[0]) || (OK_SW2 != recv[1]))
    {
        ERROR("erase df failed(%d), %02X %02X: %s", \
               len, recv[0], recv[1], iso7816_strerror(recv));
        return -1;
    }
    DEBUG("Erase DF OK");

    return 0;
}

//wstatus_upper: 添加密钥的安全状态上限
//wstatus_lower: 添加密钥的安全状态下限
int fmcos_create_keyfile(card_info_t *info, u16 id, u16 filesize, u8 shortid, u8 wstatus_upper, u8 wstatus_lower)
{
    u8 send[sizeof(cmd_apdu_t) + 1 + 7];
    cmd_apdu_t *cmd = (cmd_apdu_t *)send;
    u8 recv[2];
    int len = 0;


    cmd->cla = 0x80;
    cmd->ins = 0xE0;
    cmd->p1  = (id >> 8) & 0xFF;
    cmd->p2  = id & 0xFF;

    cmd->data[0] = 7;
    cmd->data[1] = 0x3F;
    cmd->data[2] = (filesize >> 8) & 0xFF;
    cmd->data[3] = filesize & 0xFF;
    cmd->data[4] = shortid;
    cmd->data[5] = ((wstatus_upper & 0x0F) << 4) | (wstatus_lower & 0x0F);
    cmd->data[6] = 0xFF;
    cmd->data[7] = 0xFF;
    DEBUG("Create key file, size:%u id:%04x sid:%02x perms[%x,%x]", \
           filesize, id, shortid, wstatus_upper, wstatus_lower);

    len = typeab_send(info, send, sizeof(send), recv, sizeof(recv));
    if (2 != len)
    {
        ERROR("create key file failed(%d) !", len);
        return -1;
    }
    if ((OK_SW1 != recv[0]) || (OK_SW2 != recv[1]))
    {
        ERROR("create key file failed(%d), %02X %02X: %s", \
               len, recv[0], recv[1], iso7816_strerror(recv));
        return -1;
    }
    DEBUG("Create key file OK");

    return 0;
}

int fmcos_create_binfile(card_info_t *info, u16 id, u16 filesize, u8 rstatus_upper, u8 rstatus_lower, u8 wstatus_upper, u8 wstatus_lower)
{
    u8 send[sizeof(cmd_apdu_t) + 1 + 7];
    cmd_apdu_t *cmd = (cmd_apdu_t *)send;
    u8 recv[2];
    int len = 0;


    cmd->cla = 0x80;
    cmd->ins = 0xE0;
    cmd->p1  = (id >> 8) & 0xFF;
    cmd->p2  = id & 0xFF;

    cmd->data[0] = 7;
    cmd->data[1] = 0x28;
    cmd->data[2] = (filesize >> 8) & 0xFF;
    cmd->data[3] = filesize & 0xFF;
    cmd->data[4] = ((rstatus_upper & 0x0F) << 4) | (rstatus_lower & 0x0F);
    cmd->data[5] = ((wstatus_upper & 0x0F) << 4) | (wstatus_lower & 0x0F);
    cmd->data[6] = 0xFF;
    cmd->data[7] = 0xFF;
    DEBUG("Create bin file, size:%u id:%04x rperms[%x,%x] wperms[%x,%x]", \
           filesize, id, rstatus_upper, rstatus_lower, wstatus_upper, wstatus_lower);

    len = typeab_send(info, send, sizeof(send), recv, sizeof(recv));
    if (2 != len)
    {
        ERROR("create bin file failed(%d) !", len);
        return -1;
    }
    if ((OK_SW1 != recv[0]) || (OK_SW2 != recv[1]))
    {
        ERROR("create bin file failed(%d), %02X %02X: %s", \
               len, recv[0], recv[1], iso7816_strerror(recv));
        return -1;
    }
    DEBUG("Create bin file OK");

    return 0;
}
