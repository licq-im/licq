// ICQ definitions:
#ifndef ICQDEFINES_H
#define ICQDEFINES_H

// Version constants
#define MODE_DIRECT 0x04
#define MODE_INDIRECT 0x02
#define MODE_DENIED   0x01  // user denies direct connection from "any" user

#define ICQ_VERSION 8
const unsigned short ICQ_VERSION_TCP               = 0x0006;

#define LICQ_WITHSSL     0x7D800000
#define LICQ_WITHOUTSSL  0x7D000000

// Server channels
const unsigned char ICQ_CHNxNEW                    = 0x01;
const unsigned char ICQ_CHNxDATA                   = 0x02;
const unsigned char ICQ_CHNxERROR                  = 0x03;
const unsigned char ICQ_CHNxCLOSE                  = 0x04;
const unsigned char ICQ_CHNxPING                   = 0x05;

// Server SNAC families
const unsigned short ICQ_SNACxFAM_SERVICE          = 0x0001;
const unsigned short ICQ_SNACxFAM_LOCATION         = 0x0002;
const unsigned short ICQ_SNACxFAM_BUDDY            = 0x0003;
const unsigned short ICQ_SNACxFAM_MESSAGE          = 0x0004;
const unsigned short ICQ_SNACxFAM_BOS              = 0x0009;
const unsigned short ICQ_SNACxFAM_LISTS            = 0x0013;
const unsigned short ICQ_SNACxFAM_VARIOUS          = 0x0015;

// Subtypes for service family
const unsigned short ICQ_SNACxSUB_ERROR            = 0x0001; // server
const unsigned short ICQ_SNACxSUB_READYxCLIENT     = 0x0002; // client
const unsigned short ICQ_SNACxSUB_READYxSERVER     = 0x0003; // server
const unsigned short ICQ_SNACxSUB_NEW_SERVICE      = 0x0004; // client
const unsigned short ICQ_SNACxSUB_REDIRECT         = 0x0005; // server
const unsigned short ICQ_SNACxSUB_REQ_RATE_INFO    = 0x0006; // client
const unsigned short ICQ_SNACxSUB_RATE_INFO        = 0x0007; // server
const unsigned short ICQ_SNACxSND_RATE_ACK         = 0x0008; // client

const unsigned short ICQ_SNACxSUB_RATE_CHANGE      = 0x000a; // server
const unsigned short ICQ_SNACxSUB_PAUSE            = 0x000b; // server
const unsigned short ICQ_SNACxSUB_RESUME           = 0x000d; // server
const unsigned short ICQ_SNACxSRV_GETxUSERxINFO    = 0x000e; // client
const unsigned short ICQ_SNACxRCV_NAMExINFO        = 0x000f; // server
const unsigned short ICQ_SNACxSND_NAMExINFOxACK    = 0x0011; // client
const unsigned short ICQ_SNACxSRV_IMxICQ           = 0x0017; // client
const unsigned short ICQ_SNACxSRV_ACKxIMxICQ       = 0x0018; // server
const unsigned short ICQ_SNACxSRV_SETxSTATUS       = 0x001e; // client

// Subtypes for location family
const unsigned short ICQ_SNACxLOC_REQUESTxRIGHTS   = 0x0002; // client
const unsigned short ICQ_SNAXxLOC_RIGHTSxGRANTED   = 0x0003; // server
const unsigned short ICQ_SNACxLOC_SETxUSERxINFO    = 0x0004;

// Subtypes for buddy family
const unsigned short ICQ_SNACxBDY_REQUESTxRIGHTS   = 0x0002; // client
const unsigned short ICQ_SNACxBDY_RIGHTSxGRANTED   = 0x0003; // server
const unsigned short ICQ_SNACxBDY_ADDxTOxLIST      = 0x0004; // client

// Subtypes for message family
const unsigned short ICQ_SNACxMSG_SETxICQxMODE     = 0x0002; // client
const unsigned short ICQ_SNACxMSG_REQUESTxRIGHTS   = 0x0004; // client
const unsigned short ICQ_SNAXxMSG_RIGHTSxGRANTED   = 0x0005; // server
const unsigned short ICQ_SNACxMSG_SENDxSERVER      = 0x0006; // client
const unsigned short ICQ_SNACxMSG_SERVERxMESSAGE   = 0x0007; // server
const unsigned short ICQ_SNACxSUB_ONLINExLIST      = 0x000b;
const unsigned short ICQ_SNACxSUB_OFFLINExLIST     = 0x000c;

// Subtypes for BOS family
const unsigned short ICQ_SNACxBOS_REQUESTxRIGHTS   = 0x0002; // client
const unsigned short ICQ_SNACxBOS_RIGHTSxGRANTED   = 0x0003; // client
const unsigned short ICQ_SNACxBOS_ADDxVISIBLExLIST = 0x0005;
const unsigned short ICQ_SNACxBOS_REMxVISIBLExLIST = 0x0006;
const unsigned short ICQ_SNACxBOS_ADDxINVISIBxLIST = 0x0007;
const unsigned short ICQ_SNACxBOS_REMxINVISIBxLIST = 0x0008;

// Subtypes for various family
const unsigned short ICQ_SNACxOFF_SYSMSG           = 0x0002; // client
const unsigned short ICQ_SNACxOFF_SYSMSGxACK       = 0x0002;
const unsigned short ICQ_SNACxMETA_INFO            = 0x0002; // client

// messagetypes
const unsigned short ICQ_MSGxAUTHORIZE             = 0x0008;

// UDP commands
const unsigned short ICQ_CMDxRCV_ACK               = 0x000A; //  10
const unsigned short ICQ_CMDxRCV_SERVERxDOWN       = 0x001E; //  30
const unsigned short ICQ_CMDxRCV_SETxOFFLINE       = 0x0028; //  40
const unsigned short ICQ_CMDxRCV_NEWxUIN           = 0x0046; //  70
const unsigned short ICQ_CMDxRCV_HELLO             = 0x005A; //  90
const unsigned short ICQ_CMDxRCV_WRONGxPASSWD      = 0x0064; // 100
const unsigned short ICQ_CMDxRCV_USERxONLINE       = 0x006E; // 110
const unsigned short ICQ_CMDxRCV_USERxOFFLINE      = 0x0078; // 120
const unsigned short ICQ_CMDxRCV_SEARCHxFOUND      = 0x008C; // 140
const unsigned short ICQ_CMDxRCV_SEARCHxDONE       = 0x00A0; // 160
#if ICQ_VERSION == 2
const unsigned short ICQ_CMDxRCV_UPDATEDxBASIC     = 0x00B4; // 180
const unsigned short ICQ_CMDxRCV_UPDATExBASICxFAIL = 0x00BE; // 190
#endif
const unsigned short ICQ_CMDxRCV_UPDATEDxDETAIL    = 0x00C8; // 200
const unsigned short ICQ_CMDxRCV_UPDATExDETAILxFAIL= 0x00D2; // 210
const unsigned short ICQ_CMDxRCV_SYSxMSGxOFFLINE   = 0x00DC; // 220
const unsigned short ICQ_CMDxRCV_SYSxMSGxDONE      = 0x00E6; // 230
const unsigned short ICQ_CMDxRCV_ERROR             = 0x00F0; // 240
const unsigned short ICQ_CMDxRCV_BUSY              = 0x00FA; // 250
const unsigned short ICQ_CMDxRCV_SYSxMSGxONLINE    = 0x0104; // 260
const unsigned short ICQ_CMDxRCV_USERxINFO         = 0x0118; // 280
const unsigned short ICQ_CMDxRCV_USERxDETAILS      = 0x0122; // 290
const unsigned short ICQ_CMDxRCV_USERxINVALIDxUIN  = 0x012C; // 300
const unsigned short ICQ_CMDxRCV_REVERSExTCP       = 0x015E; // 350
const unsigned short ICQ_CMDxRCV_USERxSTATUS       = 0x01A4; // 420
#if ICQ_VERSION >= 4
const unsigned short ICQ_CMDxRCV_UPDATEDxBASIC     = 0x01E0; // 480
const unsigned short ICQ_CMDxRCV_UPDATExBASICxFAIL = 0x01EA; // 490
#endif
const unsigned short ICQ_CMDxRCV_UPDATExAUTHxFAIL  = 0x01F4; // 500
const unsigned short ICQ_CMDxRCV_UPDATEDxAUTH      = 0x01FE; // 510
const unsigned short ICQ_CMDxRCV_MULTIxPACKET      = 0x0212; // 530
const unsigned short ICQ_CMDxRCV_USERxLISTxDONE    = 0x021C; // 540
const unsigned short ICQ_CMDxRCV_RANDOMxUSERxFOUND = 0x024E; // 590
const unsigned short ICQ_CMDxRCV_META              = 0x03DE; // 990

const unsigned short ICQ_CMDxSND_ACK               = 0x000A; //   10
const unsigned short ICQ_CMDxSND_THRUxSERVER       = 0x010E; //  270
const unsigned short ICQ_CMDxSND_LOGON             = 0x03E8; // 1000
const unsigned short ICQ_CMDxSND_REGISTERxUSER     = 0x03FC; // 1020
const unsigned short ICQ_CMDxSND_USERxLIST         = 0x0406; // 1030
const unsigned short ICQ_CMDxSND_SEARCHxUIN        = 0x041A; // 1050
const unsigned short ICQ_CMDxSND_SEARCHxINFO       = 0x0424; // 1060
const unsigned short ICQ_CMDxSND_PING              = 0x042E; // 1070
const unsigned short ICQ_CMDxSND_LOGOFF            = 0x0438; // 1080
const unsigned short ICQ_CMDxSND_SYSxMSGxDONExACK  = 0x0442; // 1090
const unsigned short ICQ_CMDxSND_SYSxMSGxREQ       = 0x044C; // 1100
const unsigned short ICQ_CMDxSND_AUTHORIZE         = 0x0456; // 1110
const unsigned short ICQ_CMDxSND_USERxGETINFO      = 0x0460; // 1120
const unsigned short ICQ_CMDxSND_USERxGETDETAILS   = 0x046A; // 1130
#if ICQ_VERSION == 2
const unsigned short ICQ_CMDxSND_UPDATExBASIC      = 0x04A6; // 1190
#endif
const unsigned short ICQ_CMDxSND_UPDATExDETAIL     = 0x04B0; // 1200
const unsigned short ICQ_CMDxSND_SETxSTATUS        = 0x04D8; // 1240
#if ICQ_VERSION >= 4
const unsigned short ICQ_CMDxSND_UPDATExBASIC      = 0x050A; // 1290
#endif
const unsigned short ICQ_CMDxSND_UPDATExAUTH       = 0x0514; // 1300
const unsigned short ICQ_CMDxSND_PING2             = 0x051E; // 1310
const unsigned short ICQ_CMDxSND_USERxADD          = 0x053C; // 1340
const unsigned short ICQ_CMDxSND_SETxRANDOMxCHAT   = 0x0564; // 1380
const unsigned short ICQ_CMDxSND_RANDOMxSEARCH     = 0x056E; // 1390
const unsigned short ICQ_CMDxSND_META              = 0x064A; // 1610
const unsigned short ICQ_CMDxSND_INVISIBLExLIST    = 0x06A4; // 1700
const unsigned short ICQ_CMDxSND_VISIBLExLIST      = 0x06AE; // 1710
const unsigned short ICQ_CMDxSND_MODIFYxVIEWxLIST  = 0x06B8; // 1720

// Meta commands (response)
const unsigned short ICQ_CMDxMETA_GENERALxINFOxRSP = 0x0064; // 100
const unsigned short ICQ_CMDxMETA_WORKxINFOxRSP    = 0x006E; // 110
const unsigned short ICQ_CMDxMETA_MORExINFOxRSP    = 0x0078; // 120
const unsigned short ICQ_CMDxMETA_ABOUTxRSP        = 0x0082; // 130
const unsigned short ICQ_CMDxMETA_SECURITYxRSP     = 0x00A0; // 160
const unsigned short ICQ_CMDxMETA_PASSWORDxRSP     = 0x00AA; // 170
// Meta commands (user info)
const unsigned short ICQ_CMDxMETA_GENERALxINFO     = 0x00C8; // 200
const unsigned short ICQ_CMDxMETA_WORKxINFO        = 0x00D2; // 210
const unsigned short ICQ_CMDxMETA_MORExINFO        = 0x00DC; // 220
const unsigned short ICQ_CMDxMETA_ABOUT            = 0x00E6; // 230
const unsigned short ICQ_CMDxMETA_UNKNOWNx240      = 0x00F0; // 240
const unsigned short ICQ_CMDxMETA_UNKNOWNx250      = 0x00FA; // 250
const unsigned short ICQ_CMDxMETA_BASICxINFO       = 0x0104; // 260
const unsigned short ICQ_CMDxMETA_UNKNOWNx270      = 0x010E; // 270
// Meta commands (sent)
const unsigned short ICQ_CMDxMETA_GENERALxINFOxSET = 0x03E9; // 1001
const unsigned short ICQ_CMDxMETA_WORKxINFOxSET    = 0x03F2; // 1010
const unsigned short ICQ_CMDxMETA_MORExINFOxSET    = 0x03FD; // 1020
const unsigned short ICQ_CMDxMETA_ABOUTxSET        = 0x0406; // 1030
const unsigned short ICQ_CMDxMETA_SECURITYxSET     = 0x0424; // 1060
const unsigned short ICQ_CMDxMETA_PASSWORDxSET     = 0x042E; // 1070
const unsigned short ICQ_CMDxMETA_REQUESTxALLxINFO = 0x04B1; // 1201
const unsigned short ICQ_CMDxMETA_REQUESTxBASICxINFO  = 0x04BA; // 1210
const unsigned short ICQ_CMDxMETA_REQUESTxALLxINFOx31 = 0x04CF; // 1231

// Meta commands (white page searches) - Jon
const unsigned short ICQ_CMDxMETA_SEARCHxWP           = 0x0533; // 1331
const unsigned short ICQ_CMDxMETA_SEARCHxWPxFOUND     = 0x01A4; // 388
const unsigned short ICQ_CMDxMETA_SEARCHxWPxLAST_USER = 0x01AE; // 430

const unsigned short META_SUCCESS = 0x0A;
const unsigned short META_FAILURE = 0x32;
const unsigned short META_DONE    =
                      ICQ_CMDxMETA_GENERALxINFO + ICQ_CMDxMETA_WORKxINFO +
                      ICQ_CMDxMETA_MORExINFO + ICQ_CMDxMETA_ABOUT +
                      ICQ_CMDxMETA_UNKNOWNx240 + ICQ_CMDxMETA_UNKNOWNx250 +
                      ICQ_CMDxMETA_UNKNOWNx270;

// TCP commands
const unsigned short ICQ_CMDxTCP_START             = 0x07EE;
const unsigned short ICQ_CMDxTCP_CANCEL            = 0x07D0;
const unsigned short ICQ_CMDxTCP_ACK               = 0x07DA;
const unsigned short ICQ_CMDxTCP_READxAWAYxMSG     = 0x03E8;
const unsigned short ICQ_CMDxTCP_READxOCCUPIEDxMSG = 0x03E9;
const unsigned short ICQ_CMDxTCP_READxNAxMSG       = 0x03EA;
const unsigned short ICQ_CMDxTCP_READxDNDxMSG      = 0x03EB;
const unsigned short ICQ_CMDxTCP_READxFFCxMSG      = 0x03EC;
const unsigned char  ICQ_CMDxTCP_HANDSHAKE         =   0xFF;

// Sub Commands
const unsigned short ICQ_CMDxSUB_MSG               = 0x0001;
const unsigned short ICQ_CMDxSUB_CHAT              = 0x0002;
const unsigned short ICQ_CMDxSUB_FILE              = 0x0003;
const unsigned short ICQ_CMDxSUB_URL               = 0x0004;
const unsigned short ICQ_CMDxSUB_AUTHxREQUEST      = 0x0006;
const unsigned short ICQ_CMDxSUB_AUTHxREFUSED      = 0x0007;
const unsigned short ICQ_CMDxSUB_AUTHxGRANTED      = 0x0008;
const unsigned short ICQ_CMDxSUB_ADDEDxTOxLIST     = 0x000C;
const unsigned short ICQ_CMDxSUB_WEBxPANEL         = 0x000D;
const unsigned short ICQ_CMDxSUB_EMAILxPAGER       = 0x000E;
const unsigned short ICQ_CMDxSUB_CONTACTxLIST      = 0x0013;
const unsigned short ICQ_CMDxSUB_USERxINFO         = 0x001A;  // not done
const unsigned short ICQ_CMDxSUB_FxMULTIREC        = 0x8000;
// Encryption Licq extension command
const unsigned short ICQ_CMDxSUB_SECURExOLD        = 0x00ED;  // Licq extension
const unsigned short ICQ_CMDxSUB_SECURExCLOSE      = 0x00EE;  // Licq extension
const unsigned short ICQ_CMDxSUB_SECURExOPEN       = 0x00EF;  // Licq extension


// Status constants
// Statuses must be checked in the following order:
//  DND, Occupied, NA, Away, Online
const unsigned short ICQ_STATUS_OFFLINE            = 0xFFFF;
const unsigned short ICQ_STATUS_ONLINE             = 0x0000;
const unsigned short ICQ_STATUS_AWAY               = 0x0001;
const unsigned short ICQ_STATUS_DND                = 0x0002;
const unsigned short ICQ_STATUS_NA                 = 0x0004;
const unsigned short ICQ_STATUS_OCCUPIED           = 0x0010;
const unsigned short ICQ_STATUS_FREEFORCHAT        = 0x0020;

// TCP status for ack packets
const unsigned short ICQ_TCPxACK_ONLINE            = 0x0000;
const unsigned short ICQ_TCPxACK_AWAY              = 0x0004;
const unsigned short ICQ_TCPxACK_OCCUPIED          = 0x0009;
const unsigned short ICQ_TCPxACK_DND               = 0x000A;
const unsigned short ICQ_TCPxACK_OCCUPIEDxCAR      = 0x000B;
//const unsigned short ICQ_TCPxACK_OCCUPIEDx...      = 0x000C; // received when sending to contact list when other side is occupied
const unsigned short ICQ_TCPxACK_NA                = 0x000E;
const unsigned short ICQ_TCPxACK_DNDxCAR           = 0x000F;
const unsigned short ICQ_TCPxACK_ACCEPT            = 0x0000;
const unsigned short ICQ_TCPxACK_REFUSE            = 0x0001;
// Not an official status, just something I use internally
const unsigned short ICQ_TCPxACK_RETURN            = 0x0002;

// TCP message type (composed of (status | ..._Fx...)
const unsigned short ICQ_TCPxMSG_AUTOxREPLY        = 0x0000;
const unsigned short ICQ_TCPxMSG_NORMAL            = 0x0010;
const unsigned short ICQ_TCPxMSG_LIST              = 0x0020;
const unsigned short ICQ_TCPxMSG_URGENT            = 0x0040;
const unsigned short ICQ_TCPxMSG_FxONLINE          = 0x0000;
const unsigned short ICQ_TCPxMSG_FxINVISIBLE       = 0x0080;
const unsigned short ICQ_TCPxMSG_FxAWAY            = 0x0100;
const unsigned short ICQ_TCPxMSG_FxOCCUPIED        = 0x0200;
const unsigned short ICQ_TCPxMSG_FxNA              = 0x0800;
const unsigned short ICQ_TCPxMSG_FxDND             = 0x1000;

const unsigned long ICQ_STATUS_FxFLAGS             = 0xFFFF0000;
const unsigned long ICQ_STATUS_FxUNKNOWNxFLAGS     = 0xCFC0FCC8;
const unsigned long ICQ_STATUS_FxPRIVATE           = 0x00000100;
const unsigned long ICQ_STATUS_FxPFMxAVAILABLE     = 0x00000200;  // not implemented
const unsigned long ICQ_STATUS_FxWEBxPRESENCE      = 0x00010000;
const unsigned long ICQ_STATUS_FxHIDExIP           = 0x00020000;
const unsigned long ICQ_STATUS_FxPFM               = 0x00040000;  // not implemented
const unsigned long ICQ_STATUS_FxBIRTHDAY          = 0x00080000;
const unsigned long ICQ_STATUS_FxDIRECTxDISABLED   = 0x00100000;
const unsigned long ICQ_STATUS_FxICQxHOMEPAGE      = 0x00200000;  // not implemented
const unsigned long ICQ_STATUS_FxDIRECTxLISTED     = 0x20000000;  // will accept connectio only when listed on contact list
const unsigned long ICQ_STATUS_FxDIRECTxAUTH       = 0x10000000;  // will accept connectio only when authorized


const unsigned long ICQ_RANDOMxCHATxGROUP_NONE     = 0;
const unsigned long ICQ_RANDOMxCHATxGROUP_GENERAL  = 1;
const unsigned long ICQ_RANDOMxCHATxGROUP_ROMANCE  = 2;
const unsigned long ICQ_RANDOMxCHATxGROUP_GAMES    = 3;
const unsigned long ICQ_RANDOMxCHATxGROUP_STUDENTS = 4;
const unsigned long ICQ_RANDOMxCHATxGROUP_20SOME   = 6;
const unsigned long ICQ_RANDOMxCHATxGROUP_30SOME   = 7;
const unsigned long ICQ_RANDOMxCHATxGROUP_40SOME   = 8;
const unsigned long ICQ_RANDOMxCHATxGROUP_50PLUS   = 9;
const unsigned long ICQ_RANDOMxCHATxGROUP_MxSEEKxF = 10;
const unsigned long ICQ_RANDOMxCHATxGROUP_FxSEEKxM = 11;

#endif
