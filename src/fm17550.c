#include "fm17550.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <string.h>





#define DEBUG(fmt, arg...)  do{if(pcd_debug)printf("--PCD-- " fmt "\n", ##arg);}while(0)
#define ERROR(fmt, arg...)  printf("--PCD-- " fmt "\n", ##arg)


int pcd_debug = 1;
static int fd = -1;




int fm17550_write_reg(u8 reg, u8 data)
{
    struct i2c_rdwr_ioctl_data rdwr;
    struct i2c_msg msg;
    u8 buf[2];

    buf[0] = reg;
    buf[1] = data;
    msg.addr = FM17550_I2C_ADDR;
    msg.flags = 0;
    msg.len = 2;
    msg.buf = buf;
    rdwr.msgs = &msg;
    rdwr.nmsgs = 1;
    if (1 != ioctl(fd, I2C_RDWR, &rdwr))
    {
        ERROR("write reg:0x%02x data:0x%02x failed !", reg, data);
        return -1;
    }

    return 0;
}
int fm17550_read_reg(u8 reg, u8 *data)
{
    struct i2c_rdwr_ioctl_data rdwr;
    struct i2c_msg msgs[2];

    if (NULL == data)
        return -1;

    msgs[0].addr = FM17550_I2C_ADDR;
    msgs[0].flags = 0;
    msgs[0].len = 1;
    msgs[0].buf = &reg;

    msgs[1].addr = FM17550_I2C_ADDR;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = 1;
    msgs[1].buf = data;

    rdwr.msgs = msgs;
    rdwr.nmsgs = 2;
    if (2 != ioctl(fd, I2C_RDWR, &rdwr))
    {
        ERROR("read reg:0x%02x failed !", reg);
        return -1;
    }

    return 0;
}

int fm17550_set_bits(u8 reg, u8 mask, u8 data)
{
    u8 v;

    if (0xFF == mask)
        return fm17550_write_reg(reg, data);

    if (fm17550_read_reg(reg, &v))
        return -1;

    v &= ~mask;
    v |= data & mask;
    return fm17550_write_reg(reg, v);
}

int fm17550_write_fifo(u8 *data, u16 len)
{
    struct i2c_rdwr_ioctl_data rdwr;
    struct i2c_msg msgs[1];
    u8 buf[PCD_FIFO_SIZE + 1];


    if ((NULL == data) || (PCD_FIFO_SIZE < len))
        return -1;

    buf[0] = FIFODataReg;
    memcpy(buf + 1, data, len);

    msgs[0].addr = FM17550_I2C_ADDR;
    msgs[0].flags = 0;
    msgs[0].len = len + 1;
    msgs[0].buf = buf;
/*
    msgs[1].addr = FM17550_I2C_ADDR;
    msgs[1].flags = I2C_M_NOSTART;
    msgs[1].len = len;
    msgs[1].buf = data;
*/
    rdwr.msgs = msgs;
    rdwr.nmsgs = 1;
    if (1 != ioctl(fd, I2C_RDWR, &rdwr))
    {
        ERROR("write fifo failed, len:%u", len);
        return -1;
    }

    return 0;
}
int fm17550_read_fifo(u8 *buf, u16 len)
{
    struct i2c_rdwr_ioctl_data rdwr;
    struct i2c_msg msgs[2];
    u8 reg = FIFODataReg;


    if ((NULL == buf) || (PCD_FIFO_SIZE < len))
        return -1;

    msgs[0].addr = FM17550_I2C_ADDR;
    msgs[0].flags = 0;
    msgs[0].len = 1;
    msgs[0].buf = &reg;

    msgs[1].addr = FM17550_I2C_ADDR;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = len;
    msgs[1].buf = buf;

    rdwr.msgs = msgs;
    rdwr.nmsgs = 2;
    if (2 != ioctl(fd, I2C_RDWR, &rdwr))
    {
        ERROR("read fifo failed, len:%u", len);
        return -1;
    }

    return 0;
}


int pcd_get_version(u8 *v)
{
    return fm17550_read_reg(VersionReg, v);
}

int pcd_soft_reset(void)
{
    fm17550_write_reg(CommandReg, RcvOff | CMD_SoftReset);
    usleep(1000);
    return 0;
}

int pcd_init(void)
{
    fd = open(FM17550_I2C_DEV, O_RDWR);
    if(0 > fd)
    {
        ERROR("open %s failed !", FM17550_I2C_DEV);
        return -1;
    }
    return 0;
}

int pcd_deinit(void)
{
    if (0 <= fd)
        close(fd);
    return 0;
}

int pcd_set_mode(int mode)
{
    if (PCD_MODE_HALT == mode)
    {
        fm17550_set_bits(TxControlReg, Tx2RFEn | Tx1RFEn, 0);
    }
    else if (PCD_MODE_TYPEA == mode)
    {
        fm17550_write_reg(ControlReg, ReaderMode);
        fm17550_set_bits(TxAutoReg, Force100ASK, Force100ASK);
        fm17550_write_reg(TxModeReg, TxSpeed_106K | TxFraming_14443A);
        fm17550_write_reg(RxModeReg, RxSpeed_106K | RxFraming_14443A);

        fm17550_set_bits(TxControlReg, Tx2RFEn | Tx1RFEn, Tx2RFEn | Tx1RFEn);
        usleep(100000);//调整合适时间。。。。。。。。。。。。。。。。。。。。。。。。。
    }
    else if (PCD_MODE_TYPEB == mode)
    {
        fm17550_write_reg(ControlReg, ReaderMode);
        fm17550_set_bits(TxAutoReg, Force100ASK, 0);
        fm17550_write_reg(TxModeReg, TxCRCEn | TxSpeed_106K | TxFraming_14443B);
        fm17550_write_reg(RxModeReg, RxCRCEn | RxSpeed_106K | RxFraming_14443B);
        pcd_enable_crc(1);

        //需要具体调整
        //fm17550_write_reg(RFCfgReg, RxGain_38db | RFLevel_0_17);
        fm17550_write_reg(GsNOnReg, ((0xF << CWGsNOn_SHIFT) & CWGsNOn_MASK) | (0x04 & ModGsNOn_MASK));
        fm17550_write_reg(CWGsPReg, 0x3F & CWGsP_MASK);
        //fm17550_write_reg(ModGsPReg, 0x20 & ModGsP_MASK);
        fm17550_write_reg(RxThresholdReg, ((0x08 << MinLevel_SHIFT) & MinLevel_MASK) | (0x04 & CollLevel_MASK));

        fm17550_set_bits(TxControlReg, Tx2RFEn | Tx1RFEn, Tx2RFEn | Tx1RFEn);
        usleep(100000);
    }
    else
        return -1;

    return 0;
}

int pcd_enable_crc(u8 en)
{
    fm17550_set_bits(TxModeReg, TxCRCEn, (!!en) * TxCRCEn);
    return fm17550_set_bits(RxModeReg, RxCRCEn, (!!en) * RxCRCEn);
}

int pcd_set_timer(u32 ms)
{
    u32 reload;
    u32 prescaler;

    //(ms*13560)/(2*0xFFF+1) <= 0xFFFF
    if (39586 < ms)
        return -1;

    //ms * 13560 < 0xFFFF
    if (4 >= ms)
    {
        prescaler = 0;
        reload = ms * 13560;
    }
    else
    {
        prescaler = ((ms * 13560 - 1) / 0xFFFF - 1) / 2 + 1;
        reload = ms * 13560 / (prescaler * 2 + 1);
    }

    fm17550_set_bits(TModeReg, TPrescalerHi_MASK, prescaler >> 8);
    fm17550_write_reg(TPrescalerLoReg, prescaler & 0xFF);
    fm17550_write_reg(TReloadHiReg, reload >> 8);
    fm17550_write_reg(TReloadLoReg, reload & 0xFF);

    return 0;
}


//typea模式下，coll返回冲突位置；typeb模式下返回是否有冲突(CRC错误)
int pcd_send(u8 *send, u16 send_bits, u8 *recv, u8 recv_len, u8 recv_pos, u8 *coll)
{
    u8 irq;
    u8 fifo_level = 0;
    int rst = 0;
    int i;
    u8 err, _coll, rxmode, ctrl;


    if ((NULL == send) || (0 == send_bits) || ((PCD_FIFO_SIZE * 8) < send_bits))
    {
        ERROR("%s: invalid param, send_bits:%u", __func__, send_bits);
        return -1;
    }

    if (coll)
        *coll = 0;
    if (recv_pos)
    {
        fm17550_read_reg(RxModeReg, &rxmode);
        if (RxSpeed_106K != (rxmode & RxSpeed_MASK))
        {
            ERROR("%s: recv_pos is %u but RxSpeed is not 106k !", __func__, recv_pos);
            return -1;
        }
    }

    //初始化
    fm17550_write_reg(CommandReg, RcvOff | CMD_Idle);
    fm17550_write_reg(FIFOLevelReg, FlushBuffer);
    fm17550_write_reg(WaterLevelReg, 0x20 & WaterLevel_MASK);
    fm17550_set_bits(TModeReg, TAuto, TAuto);
    fm17550_write_reg(ComIrqReg, (u8)~Set1);
    fm17550_read_reg(ComIrqReg, &irq);

    //发送
    if (recv && recv_len)
        fm17550_write_reg(CommandReg, CMD_Transceive);
    else
        fm17550_write_reg(CommandReg, CMD_Transmit);
    fm17550_write_fifo(send, (send_bits - 1) / 8 + 1);
    recv_pos = ((recv_pos & 0x07) << RxAlign_SHIFT) & RxAlign_MASK;
    send_bits = (send_bits & 0x07) & TxLastBits_MASK;
    fm17550_write_reg(BitFramingReg, StartSend | recv_pos | send_bits);

    //等待发送完成
    for (i = 0; i < 40; i++)
    {
        fm17550_read_reg(ComIrqReg, &irq);
        if(irq & TxIRq)
            break;
    }
    if (40 <= i)
    {
        ERROR("%s: wait tx timeout !", __func__);
        rst = -1;
        goto EXIT;
    }

    if ((NULL == recv) || (0 == recv_len))
    {
        rst = 0;
        goto EXIT;
    }

    //等待接收完成
    for (i = 0; i < 1700; i++)
    {
        fm17550_read_reg(ComIrqReg, &irq);
        if(irq & RxIRq)
            break;
        if (irq & TimerIRq)
        {
            DEBUG("%s: wait rx timeout irq !", __func__);
            rst = -1;
            goto EXIT;
        }
    }
    if (1700 <= i)
    {
        DEBUG("%s: wait rx timeout !", __func__);
        rst = -1;
        goto EXIT;
    }
    //读取数据
    fm17550_read_reg(FIFOLevelReg, &fifo_level);
    fm17550_read_reg(ControlReg, &ctrl);
    if (0 == fifo_level)
    {
        DEBUG("%s: no data received !", __func__);
        rst = -1;
        goto EXIT;
    }
    if (recv_len > fifo_level)
        recv_len = fifo_level;
    fm17550_read_fifo(recv, recv_len);
    rst = fifo_level * 8 - ((8 - (ctrl & RxLastBits_MASK)) & 0x07);


    //检查错误和冲突
    fm17550_read_reg(ErrorReg, &err);
    if (err & (WrErr | TempErr | BufferOvfl | ProtocolErr))
    {
        ERROR("%s: error reg 0x%02x !", __func__, err);
        rst = -1;
        goto EXIT;
    }
    fm17550_read_reg(CollReg, &_coll);
    fm17550_read_reg(RxModeReg, &rxmode);
    if (RxFraming_14443A == (rxmode & RxFraming_MASK))
    {
        if (err & CRCErr)
        {
            ERROR("%s: 14443A CRC error !", __func__);
            rst = -1;
            goto EXIT;
        }
        if (   (RxSpeed_106K == (rxmode & RxSpeed_MASK))
            && (ParityErr == (err & (CollErr | ParityErr))))
        {
            ERROR("%s: 14443A 106k mode error reg 0x%02x !", __func__, err);
            rst = -1;
            goto EXIT;
        }
        if ((ctrl & ReaderMode) && (0 == (_coll & CollPosNotValid)))
        {
            if (NULL == coll)
            {
                ERROR("%s: typea collision but coll is NULL !", __func__);
                rst = -1;
                goto EXIT;
            }
            if (_coll & CollPos_MASK)
                *coll = _coll & CollPos_MASK;
            else
                *coll = 32;
        }
    }
    else if (RxFraming_14443B == (rxmode & RxFraming_MASK))
    {
        if (err & CRCErr)
        {
            if (NULL == coll)
            {
                ERROR("%s: typeb CRC error but coll is NULL !", __func__);
                rst = -1;
                goto EXIT;
            }
            *coll = 1;
        }
    }


EXIT:
    fm17550_set_bits(ControlReg, TStopNow, TStopNow);
    fm17550_write_reg(CommandReg, CMD_Idle);
    fm17550_set_bits(BitFramingReg, StartSend, 0); //关闭发送

    return rst;
}
