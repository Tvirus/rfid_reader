#ifndef  _FM17550_H_
#define  _FM17550_H_




#ifndef u8
#define u8  unsigned char
#endif
#ifndef s8
#define s8  signed char
#endif

#ifndef u16
#define u16  unsigned short
#endif
#ifndef s16
#define s16  signed short
#endif

#ifndef u32
#define u32  unsigned int
#endif
#ifndef s32
#define s32  signed int
#endif




#define CommandReg       0x01
#define ComIEnReg        0x02
#define DivIEnReg        0x03
#define ComIrqReg        0x04
#define DivIrqReg        0x05
#define ErrorReg         0x06
#define Status1Reg       0x07
#define Status2Reg       0x08
#define FIFODataReg      0x09
#define FIFOLevelReg     0x0A
#define WaterLevelReg    0x0B
#define ControlReg       0x0C
#define BitFramingReg    0x0D
#define CollReg          0x0E
#define EXReg            0x0F

#define ModeReg          0x11
#define TxModeReg        0x12
#define RxModeReg        0x13
#define TxControlReg     0x14
#define TxAutoReg        0x15
#define TxSelReg         0x16
#define RxSelReg         0x17
#define RxThresholdReg   0x18
#define DemodReg         0x19
#define TarCardReg       0x1C
#define ManualRCVReg     0x1D
#define TypeBReg         0x1E
#define SerialSpeedReg   0x1F

#define CRCMSBReg        0x21
#define CRCLSBReg        0x22
#define GsNOffReg        0x23
#define ModWidthReg      0x24
#define TxBitPhaseReg    0x25
#define RFCfgReg         0x26
#define GsNOnReg         0x27
#define CWGsPReg         0x28
#define ModGsPReg        0x29
#define TModeReg         0x2A
#define TPrescalerLoReg  0x2B
#define TReloadHiReg     0x2C
#define TReloadLoReg     0x2D
#define TCounterValHiReg 0x2E
#define TCounterValLoReg 0x2F

#define TestSel1Reg      0x31
#define TestSel2Reg      0x32
#define TestPinEnReg     0x33
#define TestPinValueReg  0x34
#define TestBusReg       0x35
#define TestCtrlReg      0x36
#define VersionReg       0x37
#define AnalogTestReg    0x38
#define TestDAC1Reg      0x39
#define TestDAC2Reg      0x3A
#define TestADCReg       0x3B

//扩展寄存器
#define HpdCtrlEXReg     0x03
#define UseRetEXReg      0x1B
#define LVDctrlEXReg     0x1D




//CommandReg  0x01
#define RcvOff           (0x01 << 5)
#define PowerDown        (0x01 << 4)
#define Command_MASK     (0x0F)

#define CMD_Idle         0x00
#define CMD_Configure    0x01
#define CMD_RandomID     0x02
#define CMD_CalcCRC      0x03
#define CMD_Transmit     0x04
#define CMD_NoCmdChange  0x07
#define CMD_Receive      0x08
#define CMD_Transceive   0x0C
#define CMD_AutoColl     0x0D
#define CMD_M1Authent    0x0E
#define CMD_SoftReset    0x0F

//CommlEnReg  0x02
#define IrqInv           (0x01 << 7)
#define TxlEn            (0x01 << 6)
#define RxlEn            (0x01 << 5)
#define IdleIEn          (0x01 << 4)
#define HiAlertIEn       (0x01 << 3)
#define LoAlertIEn       (0x01 << 2)
#define ErrIEn           (0x01 << 1)
#define TimerIEn         (0x01 << 0)

//DivIEnReg  0x03
#define IRQPushPull      (0x01 << 7)
#define TinActIEn        (0x01 << 4)
#define CRCIEn           (0x01 << 2)
#define RfOnIEn          (0x01 << 1)
#define RfOffIEn         (0x01 << 0)

//ComIrqReg  0x04
#define Set1             (0x01 << 7)
#define TxIRq            (0x01 << 6)
#define RxIRq            (0x01 << 5)
#define IdleIRq          (0x01 << 4)
#define HiAlertIRq       (0x01 << 3)
#define LoAlertIRq       (0x01 << 2)
#define ErrIRq           (0x01 << 1)
#define TimerIRq         (0x01 << 0)
//注：清标志位时需要Set1置0、要清的位置1  !!!!!!!

//DivIrqReg  0x05
#define Set2             (0x01 << 7)
#define TinActIRq        (0x01 << 4)
#define CRCIRq           (0x01 << 2)
#define RfOnIRq          (0x01 << 1)
#define RfOffIRq         (0x01 << 0)

//ErrorReg  0x06
#define WrErr            (0x01 << 7)
#define TempErr          (0x01 << 6)
#define BufferOvfl       (0x01 << 4)
#define CollErr          (0x01 << 3) //只在106kbit按位防冲突中有效，在212和424kbit模式中始终置1
#define CRCErr           (0x01 << 2)
#define ParityErr        (0x01 << 1) //只在106kbit 14443A模式中有效
#define ProtocolErr      (0x01 << 0)

//Status1Reg  0x07
#define RFFreqOK         (0x01 << 7)
#define CRCOk            (0x01 << 6)
#define CRCReady         (0x01 << 5)
#define IRq              (0x01 << 4)
#define TRunning         (0x01 << 3)
#define RFOn             (0x01 << 2)
#define HiAlert          (0x01 << 1)
#define LoAlert          (0x01 << 0)

//Status2Reg  0x08
#define TempSensClear    (0x01 << 7)
#define I2CForceHS       (0x01 << 6)
#define CardActiveted    (0x01 << 4)
#define Crypto1On        (0x01 << 3)
#define ModemState_MASK  (0x07)
#define ModemState_Idle      (0x00)
#define ModemState_WaitStart (0x01)
#define ModemState_TxWait    (0x02)
#define ModemState_Sending   (0x03)
#define ModemState_RxWait    (0x04)
#define ModemState_WaitData  (0x05)
#define ModemState_Recving   (0x06)

//FIFOLevelReg  0x0A
#define FlushBuffer      (0x01 << 7)
#define FIFOLevel_MASK   (0x7F)

//WaterLevelReg  0x0B
#define WaterLevel_MASK  (0x3F)

//ControlReg  0x0C
#define TStopNow         (0x01 << 7)
#define TStartNow        (0x01 << 6)
#define WrCardIDtoFIFO   (0x01 << 5)
#define ReaderMode       (0x01 << 4)
#define RxLastBits_MASK  (0x07)

//BitFramingReg  0x0D
#define StartSend        (0x01 << 7)
#define RxAlign_SHIFT    (0x04)
#define RxAlign_MASK     (0x07 << RxAlign_SHIFT) //只在106kbit按位防冲突中有效，其他模式下置0
#define TxLastBits_MASK  (0x07)

//CollReg  0x0E
#define ValuesAfterColl  (0x01 << 7) //只在106kbit按位防冲突中使用，其他模式下置1
#define CollPosNotValid  (0x01 << 5) //只在14443A读写器模式中被解读
#define CollPos_MASK     (0x1F) //只在14443A读写器模式并且CollPosNotValid为1时被解读

//EXReg  0x0F
#define EXmode_MASK      (0xC0)
#define EXmode_WrAddr    (0x40)
#define EXmode_RdAddr    (0x80)
#define EXmode_WrData    (0xC0)
#define EXmode_RdData    (0x00)
#define EXAddr_MASK      (0x3F)


//ModeReg  0x11
#define MSBFirst         (0x01 << 7)
#define TxWaitRF         (0x01 << 5)
#define RxWaitRF         (0x01 << 4)
#define PolTin           (0x01 << 3)
#define CRCPreset_MASK   (0x03)
#define CRCPreset_0000   (0x00)
#define CRCPreset_6363   (0x01)
#define CRCPreset_A671   (0x02)
#define CRCPreset_FFFF   (0x03)

//TxModeReg  0x12
#define TxCRCEn          (0x01 << 7) //只能在106kbit模式下置0
#define TxSpeed_MASK     (0x70)
#define TxSpeed_106K     (0x00)
#define TxSpeed_212K     (0x10)
#define TxSpeed_424K     (0x20)
#define InvMod           (0x01 << 3)
#define TxMix            (0x01 << 2)
#define TxFraming_MASK   (0x03)
#define TxFraming_14443A (0x00)
#define TxFraming_14443B (0x03)

//RxModeReg  0x13
#define RxCRCEn          (0x01 << 7) //只能在106kbit模式下置0
#define RxSpeed_MASK     (0x70)
#define RxSpeed_106K     (0x00)
#define RxSpeed_212K     (0x10)
#define RxSpeed_424K     (0x20)
#define RxNoErr          (0x01 << 3)
#define RxMultiple       (0x01 << 2)
#define RxFraming_MASK   (0x03)
#define RxFraming_14443A (0x00)
#define RxFraming_14443B (0x03)

//TxControlReg  0x14
#define InvTx2RFOn       (0x01 << 7)
#define InvTx1RFOn       (0x01 << 6)
#define InvTx2RFOff      (0x01 << 5)
#define InvTx1RFOff      (0x01 << 4)
#define Tx2CW            (0x01 << 3)
#define Tx2RFEn          (0x01 << 1)
#define Tx1RFEn          (0x01 << 0)

//TxAutoReg  0x15
#define Force100ASK      (0x01 << 6)
#define AutoWakeUp       (0x01 << 5)

//TxSelReg  0x16
#define DriverSel_MASK   (0x30)
#define DriverSel_3State (0x00)
#define DriverSel_Encode (0x10)
#define DriverSel_Tin    (0x20)
#define DriverSel_High   (0x30)
#define TOutSel_MASK     (0x0F)
#define TOutSel_3State   (0x00)
#define TOutSel_Low      (0x01)
#define TOutSel_High     (0x02)
#define TOutSel_TestBus  (0x03)
#define TOutSel_Encode   (0x04)
#define TOutSel_SendData (0x05)
#define TOutSel_Receiver (0x06)
#define TOutSel_RecvedData (0x07)
#define TOutSel_RXData   (0x0C)
#define TOutSel_TXData   (0x0D)
#define TOutSel_RXNoFilter (0x0E)
#define TOutSel_RXEnvelope (0x0F)

//RxSelReg  0x17
#define UartSel_MASK     (0xC0)
#define UartSel_Low      (0x00)
#define UartSel_TinEnvelope (0x40)
#define UartSel_IntSignal (0x80)
#define UartSel_TinModulating (0xC0)
#define RxWait_MASK     (0x3F)

//RxThresholdReg  0x18
#define MinLevel_SHIFT   (0x04)
#define MinLevel_MASK    (0x0F << MinLevel_SHIFT)
#define CollLevel_MASK   (0x07)

//DemodReg  0x19
#define AddIQ_MASK       (0xC0)
#define AddIQ_Strong     (0x00)
#define AddIQ_StrongKeep (0x40)
#define AddIQ_IQ         (0x80)
#define AddIQ_FixIQ_KeepI (0x00)
#define AddIQ_FixIQ_KeepQ (0x40)
#define FixIQ            (0x01 << 5)
#define TypeBEOFMode     (0x01 << 4)
#define TauRcv_SHIFT     (0x02)
#define TauRcv_MASK      (0x03 << TauRcv_SHIFT)
#define TauSync_MASK     (0x03)

//TarCardReg  0x1C
#define SensMiller_SHIFT (0x05)
#define SensMiller_MASK  (0x07 << SensMiller_SHIFT)
#define TauMiller_SHIFT  (0x03)
#define TauMiller_MASK   (0x03 << TauMiller_SHIFT)
#define AHalted          (0x01 << 2)
#define TxWait_MASK      (0x03)

//ManualRCVReg  0x1D
#define FastFiltSO       (0x01 << 6)
#define ParityDisable    (0x01 << 4)
#define LargeBWPLL       (0x01 << 3)
#define ManualHPCF       (0x01 << 2)
#define HPCF_MASK        (0x03)
#define HPCF_106K        (0x00)
#define HPCF_212K        (0x01)
#define HPCF_424K        (0x02)
#define HPCF_848K        (0x03)

//TypeBReg  0x1E
#define RxSOFReq         (0x01 << 7)
#define RxEOFReq         (0x01 << 6)
#define EOFSOFWidth      (0x01 << 4)
#define NoTxSOF          (0x01 << 3)
#define NoTxEOF          (0x01 << 2)
#define TxEGT_MASK       (0x03)
#define TxEGT_0bit       (0x00)
#define TxEGT_2bit       (0x01)
#define TxEGT_4bit       (0x02)
#define TxEGT_6bit       (0x03)

//SerialSpeedReg  0x1F
#define BR_T0_SHIFT      (0x05)
#define BR_T0_MASK       (0x07 << BR_T0_SHIFT)
#define BR_T1_MASK       (0x0F)


//GsNOffReg  0x23
#define CWGsNOff_SHIFT   (0x04)
#define CWGsNOff_MASK    (0x0F << CWGsNOff_SHIFT)
#define ModGsNOff_MASK   (0x0F)

//TxBitPhaseReg  0x25
#define RcvClkChange     (0x01 << 7)
#define TxBitPhase_MASK  (0x7F)

//RFCfgReg  0x26
#define RxGain_MASK      (0x70)
#define RxGain_18db      (0x00)
#define RxGain_23db      (0x10)
#define RxGain_18db_     (0x20)
#define RxGain_23db_     (0x30)
#define RxGain_33db      (0x40)
#define RxGain_38db      (0x50)
#define RxGain_43db      (0x60)
#define RxGain_48db      (0x70)
#define RFLevel_MASK     (0x0F)
#define RFLevel_2        (0x0F)
#define RFLevel_1_4      (0x0E)
#define RFLevel_0_99     (0x0D)
#define RFLevel_0_69     (0x0C)
#define RFLevel_0_49     (0x0B)
#define RFLevel_0_35     (0x0A)
#define RFLevel_0_24     (0x09)
#define RFLevel_0_17     (0x08)
#define RFLevel_0_12     (0x07)
#define RFLevel_0_083    (0x06)
#define RFLevel_0_058    (0x05)
#define RFLevel_0_041    (0x04)
#define RFLevel_0_029    (0x03)
#define RFLevel_0_020    (0x02)
#define RFLevel_0_014    (0x01)
#define RFLevel_0_010    (0x00)

//GsNOnReg  0x27
#define CWGsNOn_SHIFT    (0x04)
#define CWGsNOn_MASK     (0x0F << CWGsNOn_SHIFT)
#define ModGsNOn_MASK    (0x0F)

//CWGsPReg  0x28
#define CWGsP_MASK       (0x3F)

//ModGsPReg  0x29
#define ModGsP_MASK      (0x3F)

//TModeReg  0x2A
#define TAuto            (0x01 << 7)
#define TGated_MASK      (0x60)
#define TGated_Non       (0x00)
#define TGated_Tin       (0x20)
#define TGated_AUX1      (0x40)
#define TAutoRestart     (0x01 << 4)
#define TPrescalerHi_MASK (0x0F)






#define PCD_FIFO_SIZE  64

#define FM17550_I2C_DEV   "/dev/i2c-4"
#define FM17550_I2C_ADDR  0x28


extern int fm17550_debug;


extern int pcd_init(void);
extern int pcd_get_version(u8 *v);
extern int pcd_soft_reset(void);
extern int pcd_deinit(void);
#define PCD_MODE_HALT   0
#define PCD_MODE_TYPEA  1
#define PCD_MODE_TYPEB  2
extern int pcd_set_mode(int mode);
extern int pcd_enable_crc(u8 en);
extern int pcd_set_timer(u32 ms);
extern int pcd_send(u8 *send, u16 send_bits, u8 *recv, u8 recv_len, u8 recv_pos, u8 *coll_pos);
extern int pcd_send_short(u8 cmd, u8 *recv, u8 recv_len);


extern int fm17550_write_reg(u8 reg, u8 data);
extern int fm17550_read_reg(u8 reg, u8 *data);
extern int fm17550_set_bits(u8 reg, u8 mask, u8 data);
extern int fm17550_write_fifo(u8 *data, u16 len);
extern int fm17550_read_fifo(u8 *buf, u16 len);


#endif
