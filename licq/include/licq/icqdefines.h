/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2010 Licq developers
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

// ICQ definitions:
#ifndef LICQ_ICQDEFINES_H
#define LICQ_ICQDEFINES_H

// Version constants
#define MODE_DIRECT 0x04
#define MODE_INDIRECT 0x02
#define MODE_DENIED   0x01  // user denies direct connection from "any" user

#define ICQ_VERSION 8
const unsigned short ICQ_VERSION_TCP               = 0x0008;

#define LICQ_WITHSSL     0x7D800000
#define LICQ_WITHOUTSSL  0x7D000000

// Easy to compare snacs
#define MAKESNAC(fam, subtype)  ((fam << 16) | (subtype))

// Server channels
const unsigned char ICQ_CHNxNEW                    = 0x01;
const unsigned char ICQ_CHNxDATA                   = 0x02;
const unsigned char ICQ_CHNxERROR                  = 0x03;
const unsigned char ICQ_CHNxCLOSE                  = 0x04;
const unsigned char ICQ_CHNxPING                   = 0x05;
// Licq internal
const unsigned char ICQ_CHNxNONE                   = 0xFC;
const unsigned char ICQ_CHNxINFO                   = 0xFD;
const unsigned char ICQ_CHNxSTATUS                 = 0xFE;
const unsigned char ICQ_CHNxUNKNOWN                = 0xFF;

// Server SNAC families
const unsigned short ICQ_SNACxFAM_SERVICE          = 0x0001;
const unsigned short ICQ_SNACxFAM_LOCATION         = 0x0002;
const unsigned short ICQ_SNACxFAM_BUDDY            = 0x0003;
const unsigned short ICQ_SNACxFAM_MESSAGE          = 0x0004;
const unsigned short ICQ_SNACxFAM_BOS              = 0x0009;
const unsigned short ICQ_SNACxFAM_BART             = 0x0010;
const unsigned short ICQ_SNACxFAM_LIST             = 0x0013;
const unsigned short ICQ_SNACxFAM_VARIOUS          = 0x0015;
const unsigned short ICQ_SNACxFAM_AUTH             = 0x0017;

// Subtypes for service family
const unsigned short ICQ_SNACxSUB_ERROR            = 0x0001; // server
const unsigned short ICQ_SNACxSUB_READYxCLIENT     = 0x0002; // client
const unsigned short ICQ_SNACxSUB_READYxSERVER     = 0x0003; // server
const unsigned short ICQ_SNACxSUB_NEW_SERVICE      = 0x0004; // client
const unsigned short ICQ_SNACxSUB_REDIRECT         = 0x0005; // server
const unsigned short ICQ_SNACxSUB_REQ_RATE_INFO    = 0x0006; // client
const unsigned short ICQ_SNACxSUB_RATE_INFO        = 0x0007; // server
const unsigned short ICQ_SNACxSND_RATE_ACK         = 0x0008; // client
const unsigned short ICQ_SNACxSUB_RATE_WARNING     = 0x000a; // server
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
const unsigned short ICQ_SNACxLOC_INFOxREQ         = 0x0005; // client
const unsigned short ICQ_SNACxREPLYxUSERxINFO      = 0x0006; // client
const unsigned short ICQ_SNACxREQUESTxUSERxINFO    = 0x0015; // client

// Subtypes for buddy family
const unsigned short ICQ_SNACxBDY_REQUESTxRIGHTS   = 0x0002; // client
const unsigned short ICQ_SNACxBDY_RIGHTSxGRANTED   = 0x0003; // server
const unsigned short ICQ_SNACxBDY_ADDxTOxLIST      = 0x0004; // client
const unsigned short ICQ_SNACxBDY_REMOVExFROMxLIST = 0x0005; // client
const unsigned short ICQ_SNACxSUB_ONLINExLIST      = 0x000b;
const unsigned short ICQ_SNACxSUB_OFFLINExLIST     = 0x000c;

// Subtypes for message family
const unsigned short ICQ_SNACxMSG_ICBMxERROR       = 0x0001; // server
const unsigned short ICQ_SNACxMSG_SETxICQxMODE     = 0x0002; // client
const unsigned short ICQ_SNACxMSG_REQUESTxRIGHTS   = 0x0004; // client
const unsigned short ICQ_SNACxMSG_RIGHTSxGRANTED   = 0x0005; // server
const unsigned short ICQ_SNACxMSG_SENDxSERVER      = 0x0006; // client
const unsigned short ICQ_SNACxMSG_SERVERxMESSAGE   = 0x0007; // server
const unsigned short ICQ_SNACxMSG_SERVERxREPLYxMSG = 0x000b; // client, server
const unsigned short ICQ_SNACxMSG_SERVERxACK       = 0x000c; // server
const unsigned short ICQ_SNACxMSG_TYPING           = 0x0014; // client, server
// Subtypes for BOS family
const unsigned short ICQ_SNACxBOS_REQUESTxRIGHTS   = 0x0002; // client
const unsigned short ICQ_SNACxBOS_RIGHTSxGRANTED   = 0x0003; // client
const unsigned short ICQ_SNACxBOS_ADDxVISIBLExLIST = 0x0005;
const unsigned short ICQ_SNACxBOS_REMxVISIBLExLIST = 0x0006;
const unsigned short ICQ_SNACxBOS_ADDxINVISIBxLIST = 0x0007;
const unsigned short ICQ_SNACxBOS_REMxINVISIBxLIST = 0x0008;

// Subtypes for various family
//const unsigned short ICQ_SNACxOFF_SYSMSG         = 0x0002; // client
//const unsigned short ICQ_SNACxOFF_SYSMSGxACK     = 0x0002;
const unsigned short ICQ_SNACxMETA                 = 0x0002; // client
//const unsigned short ICQ_SNACxSEARCH             = 0x0002; // client

// Subtypes for new uin family
const unsigned short ICQ_SNACxNEW_UIN_ERROR        = 0x0001; // server
const unsigned short ICQ_SNACxAUTHxLOGON           = 0x0002; // client
const unsigned short ICQ_SNACxAUTHxLOGON_REPLY     = 0x0003; // server
const unsigned short ICQ_SNACxREGISTER_USER        = 0x0004; // client
const unsigned short ICQ_SNACxNEW_UIN              = 0x0005; // server
const unsigned short ICQ_SNACxAUTHxREQUEST_SALT    = 0x0006; // client
const unsigned short ICQ_SNACxAUTHxSALT_REPLY      = 0x0007; // server
const unsigned short ICQ_SNACxREQUEST_IMAGE        = 0x000C; // client
const unsigned short ICQ_SNACxSEND_IMAGE           = 0x000D; // server

// Subtypes for BART family
const unsigned short ICQ_SNACxBART_ERROR           = 0x0001; // server
const unsigned short ICQ_SNACxBART_UPLOAD          = 0x0002; // client
const unsigned short ICQ_SNACxBART_UPLOADxACK      = 0x0003; // server
const unsigned short ICQ_SNACxBART_DOWNLOADxREQUEST= 0x0006; // client
const unsigned short ICQ_SNACxBART_DOWNLOADxREPLY  = 0x0007; // server

// Subtypes for list family
const unsigned short ICQ_SNACxLIST_REQUESTxRIGHTS  = 0x0002; // client
const unsigned short ICQ_SNACxLIST_RIGHTSxGRANTED  = 0x0003; // server
const unsigned short ICQ_SNACxLIST_REQUESTxROST2   = 0x0004; // server
const unsigned short ICQ_SNACxLIST_REQUESTxROST    = 0x0005; // client
const unsigned short ICQ_SNACxLIST_ROSTxREPLY      = 0x0006; // server
const unsigned short ICQ_SNACxLIST_ROSTxACK        = 0x0007; // client
const unsigned short ICQ_SNACxLIST_ROSTxADD        = 0x0008; // client
const unsigned short ICQ_SNACxLIST_ROSTxUPD_GROUP  = 0x0009; // client
const unsigned short ICQ_SNACxLIST_ROSTxREM        = 0x000A; // client
const unsigned short ICQ_SNACxLIST_UPDxACK         = 0x000E; // server
const unsigned short ICQ_SNACxLIST_ROSTxSYNCED     = 0x000F; // server
const unsigned short ICQ_SNACxLIST_ROSTxEDITxSTART = 0x0011; // client, server
const unsigned short ICQ_SNACxLIST_ROSTxEDITxEND   = 0x0012; // client, server
const unsigned short ICQ_SNACxLIST_AUTHxREQ2       = 0x0014; // client
const unsigned short ICQ_SNACxLIST_AUTHxREQ        = 0x0018; // client
const unsigned short ICQ_SNACxLIST_AUTHxREQxSRV    = 0x0019; // server
const unsigned short ICQ_SNACxLIST_AUTHxGRANT      = 0x001A; // client
const unsigned short ICQ_SNACxLIST_AUTHxRESPONS    = 0x001B; // server
const unsigned short ICQ_SNACxLIST_AUTHxADDED      = 0x001C; // server

// messagetypes
const unsigned short ICQ_MSGxAUTHORIZE             = 0x0008;

// Roster Types
const unsigned short ICQ_ROSTxNORMAL               = 0x0000;
const unsigned short ICQ_ROSTxGROUP                = 0x0001;
const unsigned short ICQ_ROSTxVISIBLE              = 0x0002;
const unsigned short ICQ_ROSTxINVISIBLE            = 0x0003;
const unsigned short ICQ_ROSTxPDINFO               = 0x0004;
const unsigned short ICQ_ROSTxICQTIC               = 0x0009;
const unsigned short ICQ_ROSTxIGNORE               = 0x000E;
const unsigned short ICQ_ROSTxIMPORT               = 0x0013;

//********* OLD COMMANDS ******************************************

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
const unsigned short ICQ_CMDxSND_SYSxMSGxDONExACK  = 0x003E; // was 1090
const unsigned short ICQ_CMDxSND_SYSxMSGxREQ       = 0x003C; // was 1100
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
const unsigned short ICQ_CMDxMETA_EMAILxINFOxRSP   = 0x0087; // 140
const unsigned short ICQ_CMDxMETA_INTERESTSxINFOxRSP = 0x008C; // 140
const unsigned short ICQ_CMDxMETA_ORGBACKxINFOxRSP = 0x0096; // 150
const unsigned short ICQ_CMDxMETA_SENDxSMSxRSP     = 0x0096; // 150
// Deprecated? const unsigned short ICQ_CMDxMETA_SECURITYxRSP     = 0x00A0; // 160
const unsigned short ICQ_CMDxMETA_SECURITYxRSP     = 0x0C3F;
const unsigned short ICQ_CMDxMETA_PASSWORDxRSP     = 0x00AA; // 170
const unsigned short ICQ_CMDxMETA_RANDOMxUSERxRSP  = 0x0366; // 870
const unsigned short ICQ_CMDxMETA_SETxRANDxCHATxRSP= 0x0370; // 880
// Meta commands (user info)
const unsigned short ICQ_CMDxMETA_GENERALxINFO     = 0x00C8; // 200
const unsigned short ICQ_CMDxMETA_WORKxINFO        = 0x00D2; // 210
const unsigned short ICQ_CMDxMETA_MORExINFO        = 0x00DC; // 220
const unsigned short ICQ_CMDxMETA_ABOUT            = 0x00E6; // 230
const unsigned short ICQ_CMDxMETA_EMAILxINFO       = 0x00EB; // 235
const unsigned short ICQ_CMDxMETA_INTERESTSxINFO   = 0x00F0; // 240
const unsigned short ICQ_CMDxMETA_PASTxINFO        = 0x00FA; // 250
const unsigned short ICQ_CMDxMETA_BASICxINFO       = 0x0104; // 260
const unsigned short ICQ_CMDxMETA_HOMEPAGExINFO    = 0x010E; // 270
// Meta commands (sent)
const unsigned short ICQ_CMDxMETA_GENERALxINFOxSET = 0x03EA; // 1002
const unsigned short ICQ_CMDxMETA_WORKxINFOxSET    = 0x03F3; // 1011
const unsigned short ICQ_CMDxMETA_MORExINFOxSET    = 0x03FD; // 1021
const unsigned short ICQ_CMDxMETA_ABOUTxSET        = 0x0406; // 1030
const unsigned short ICQ_CMDxMETA_EMAILxINFOxSET   = 0x040B; // 1035
const unsigned short ICQ_CMDxMETA_INTERESTSxINFOxSET = 0x0410; // 1040
const unsigned short ICQ_CMDxMETA_ORGBACKxINFOxSET = 0x041A; // 1050
// Deprecated? const unsigned short ICQ_CMDxMETA_SECURITYxSET     = 0x0424; // 1060
const unsigned short ICQ_CMDxMETA_SECURITYxSET     = 0x0C3A;
const unsigned short ICQ_CMDxMETA_PASSWORDxSET     = 0x042E; // 1070
const unsigned short ICQ_CMDxMETA_REQUESTxALLxINFO = 0x04B2; // 1202
const unsigned short ICQ_CMDxMETA_REQUESTxBASICxINFO  = 0x04BA; // 1210
const unsigned short ICQ_CMDxMETA_REQUESTxALLxINFOx31 = 0x04CF; // 1231
const unsigned short ICQ_CMDxMETA_REQUESTxALLxINFOxOWNER = 0x04D0; // 1232
const unsigned short ICQ_CMDxMETA_SEARCHxWP        = 0x055F;
const unsigned short ICQ_CMDxMETA_SEARCHxUIN       = 0x0569;
const unsigned short ICQ_CMDxMETA_RANDOMxSEARCH    = 0x074E; // 1870
const unsigned short ICQ_CMDxMETA_SETxRANDOMxCHAT  = 0x0758; // 1880
const unsigned short ICQ_CMDxMETA_SENDxSMS         = 0x1482;
const unsigned short ICQ_CMDxMETA_WPxINFOxSET      = 0x0C3A;
const unsigned short ICQ_CMDxMETA_WPxINFOxSET_RSP  = 0x0C3F;

// Meta commands (white page searches) - Jon
//const unsigned short ICQ_CMDxMETA_SEARCHxWP           = 0x0533; // 1331
const unsigned short ICQ_CMDxMETA_SEARCHxWPxFOUND     = 0x01A4; // 388
const unsigned short ICQ_CMDxMETA_SEARCHxWPxLAST_USER = 0x01AE; // 430

// Whitepage Info Headers (BE)
const unsigned short ICQ_CMDxWPxWEB_PRESENCE    = 0x0C03;
const unsigned short ICQ_CMDxWPxHOMEPAGE        = 0x1302;
const unsigned short ICQ_CMDxWPxTIMEZONE        = 0x1603;
const unsigned short ICQ_CMDxWPxBIRTHDAY        = 0x3A02;
const unsigned short ICQ_CMDxWPxFNAME           = 0x4001;
const unsigned short ICQ_CMDxWPxLNAME           = 0x4A01;
const unsigned short ICQ_CMDxWPxRECV_ICQ_SPAM   = 0x4803;
const unsigned short ICQ_CMDxWPxALIAS           = 0x5401;
const unsigned short ICQ_CMDxWPxABOUT           = 0x5802;
const unsigned short ICQ_CMDxWPxEMAIL           = 0x5E01;
const unsigned short ICQ_CMDxWPxHOME_ADDR       = 0x6201;
const unsigned short ICQ_CMDxWPxAGE             = 0x6801;
const unsigned short ICQ_CMDxWPxHOME_ZIP        = 0x6D02;
const unsigned short ICQ_CMDxWPxAGE2            = 0x7201;
const unsigned short ICQ_CMDxWPxPHONExHOME      = 0x7602;
const unsigned short ICQ_CMDxWPxGENDER          = 0x7C01;
const unsigned short ICQ_CMDxWPxHOME_FAX        = 0x8002;
const unsigned short ICQ_CMDxWPxLANGUAGE        = 0x8601;
const unsigned short ICQ_CMDxWPxPHONExCELL      = 0x8A02;
const unsigned short ICQ_CMDxWPxCITY            = 0x9001;
const unsigned short ICQ_CMDxWPxWORK_ADDR       = 0x9402;
const unsigned short ICQ_CMDxWPxSTATE           = 0x9A01;
const unsigned short ICQ_CMDxWPxWORK_CITY       = 0x9E02;
const unsigned short ICQ_CMDxWPxCOUNTRY         = 0xA401;
const unsigned short ICQ_CMDxWPxWORK_STATE      = 0xA802;
const unsigned short ICQ_CMDxWPxCOMPANY         = 0xAE01;
const unsigned short ICQ_CMDxWPxWORK_COUNTRY    = 0xB202;
const unsigned short ICQ_CMDxWPxCODEPT          = 0xB801;
const unsigned short ICQ_CMDxWPxWORK_ZIP        = 0xBD02;
const unsigned short ICQ_CMDxWPxCOPOS           = 0xC201;
const unsigned short ICQ_CMDxWPxWORK_PHONE      = 0xC602;
const unsigned short ICQ_CMDxWPxOCCUP           = 0xCC01;
const unsigned short ICQ_CMDxWPxWORK_FAX        = 0xD002;
const unsigned short ICQ_CMDxWPxWORK_URL        = 0xDA02;
const unsigned short ICQ_CMDxWPxPASTAFF         = 0xD601;
const unsigned short ICQ_CMDxWPxORGANIZ         = 0xFE01;
const unsigned short ICQ_CMDxWPxINTEREST        = 0xEA01;
const unsigned short ICQ_CMDxWPxAUTH            = 0xF802;
const unsigned short ICQ_CMDxWPxKEYWORD         = 0x2602;
const unsigned short ICQ_CMDxWPxONLINE          = 0x3002;


const unsigned short META_SUCCESS = 0x0A;
const unsigned short META_FAILURE = 0x32;
// const unsigned short META_DONE    =
//                       ICQ_CMDxMETA_GENERALxINFO + ICQ_CMDxMETA_WORKxINFO +
//                       ICQ_CMDxMETA_MORExINFO + ICQ_CMDxMETA_ABOUT +
//                       ICQ_CMDxMETA_UNKNOWNx240 + ICQ_CMDxMETA_UNKNOWNx250 +
//                       ICQ_CMDxMETA_UNKNOWNx270;

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
const unsigned short ICQ_CMDxSUB_MSGxSERVER        = 0x0009;
const unsigned short ICQ_CMDxSUB_EXTERNAL          = 0x000A;
const unsigned short ICQ_CMDxSUB_ADDEDxTOxLIST     = 0x000C;
const unsigned short ICQ_CMDxSUB_WEBxPANEL         = 0x000D;
const unsigned short ICQ_CMDxSUB_EMAILxPAGER       = 0x000E;
const unsigned short ICQ_CMDxSUB_CONTACTxLIST      = 0x0013;
const unsigned short ICQ_CMDxSUB_PHONExCALL        = 0x0014;
const unsigned short ICQ_CMDxSUB_PHONExBOOK        = 0x0016;
const unsigned short ICQ_CMDxSUB_PICTURE           = 0x0019;
const unsigned short ICQ_CMDxSUB_SMS               = 0x001A;
const unsigned short ICQ_CMDxSUB_ICBM              = 0x001A;  // This one sucks
const unsigned short ICQ_CMDxSUB_FxMULTIREC        = 0x8000;
// These exist as ICBM plugins only and the number is unofficial
const unsigned short ICQ_CMDxSUB_SENDxEICQ         = 0x00D7;
const unsigned short ICQ_CMDxSUB_CONACTxREQUEST    = 0x00D8;
const unsigned short ICQ_CMDxSUB_ACTIVExLIST       = 0x00D9;
const unsigned short ICQ_CMDxSUB_PCxTOxPCxCALL     = 0x00DA;
const unsigned short ICQ_CMDxSUB_GREETINGxCARD     = 0x00DB;
const unsigned short ICQ_CMDxSUB_VOICExMESSAGE     = 0x00DC;
const unsigned short ICQ_CMDxSUB_IRCQxNET          = 0x00DD;
const unsigned short ICQ_CMDxSUB_HOMEPAGExINVITE   = 0x00DE;
const unsigned short ICQ_CMDxSUB_HOMEPAGExNEWS     = 0x00DF;
// Created for protocol plugins
const unsigned short ICQ_CMDxSUB_EMAILxALERT       = 0x00EC;
// Encryption Licq extension command
const unsigned short ICQ_CMDxSUB_SECURExOLD        = 0x00ED;  // Licq extension
const unsigned short ICQ_CMDxSUB_SECURExCLOSE      = 0x00EE;  // Licq extension
const unsigned short ICQ_CMDxSUB_SECURExOPEN       = 0x00EF;  // Licq extension


// Special UINs
const unsigned short ICQ_UINxPAGER              = 0x000A;
const unsigned short ICQ_UINxSMS                   = 0x03EA;

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
const unsigned short ICQ_TCPxACK_OCCUPIEDx2        = 0x000C; // received when sending to contact list when other side is occupied
const unsigned short ICQ_TCPxACK_NA                = 0x000E;
const unsigned short ICQ_TCPxACK_DNDxCAR           = 0x000F;
const unsigned short ICQ_TCPxACK_ACCEPT            = 0x0000;
const unsigned short ICQ_TCPxACK_REFUSE            = 0x0001;
// Not an official status, just something I use internally
const unsigned short ICQ_TCPxACK_RETURN            = 0x0002;

// TCP message type (composed of (status | ..._Fx...)
const unsigned short ICQ_TCPxMSG_AUTOxREPLY        = 0x0000;
const unsigned short ICQ_TCPxMSG_NORMAL            = 0x0010;
const unsigned short ICQ_TCPxMSG_NORMAL2           = 0x0001;
const unsigned short ICQ_TCPxMSG_LIST              = 0x0020;
const unsigned short ICQ_TCPxMSG_LIST2             = 0x0004;
const unsigned short ICQ_TCPxMSG_URGENT            = 0x0040;
const unsigned short ICQ_TCPxMSG_URGENT2           = 0x0002;
const unsigned short ICQ_TCPxMSG_FxONLINE          = 0x0000;
const unsigned short ICQ_TCPxMSG_FxINVISIBLE       = 0x0080;
const unsigned short ICQ_TCPxMSG_FxAWAY            = 0x0100;
const unsigned short ICQ_TCPxMSG_FxOCCUPIED        = 0x0200;
const unsigned short ICQ_TCPxMSG_FxNA              = 0x0800;
const unsigned short ICQ_TCPxMSG_FxDND             = 0x1000;

const unsigned long ICQ_STATUS_FxFLAGS             = 0xFFFF0000;
const unsigned long ICQ_STATUS_FxUNKNOWNxFLAGS     = 0xCFC0FCC8;
const unsigned long ICQ_STATUS_FxPRIVATE           = 0x00000100;
const unsigned long ICQ_STATUS_FxPFMxAVAILABLE     = 0x00000200;
const unsigned long ICQ_STATUS_FxWEBxPRESENCE      = 0x00010000;
const unsigned long ICQ_STATUS_FxHIDExIP           = 0x00020000;
const unsigned long ICQ_STATUS_FxPFM               = 0x00040000;
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

const unsigned short ICQ_TYPING_INACTIVEx0        = 0; //icq2go sends this
const unsigned short ICQ_TYPING_INACTIVEx1        = 1; //icqlite sends this
const unsigned short ICQ_TYPING_INACTIVE          = 1; //for plugins
const unsigned short ICQ_TYPING_ACTIVE            = 2;

// Privacy settings
const unsigned char ICQ_PRIVACY_ALLOW_ALL         = 1;
const unsigned char ICQ_PRIVACY_BLOCK_ALL         = 2;
const unsigned char ICQ_PRIVACY_ALLOW_FOLLOWING   = 3; //used for invisible
const unsigned char ICQ_PRIVACY_BLOCK_FOLLOWING   = 4;
const unsigned char ICQ_PRIVACY_ALLOW_ONLY_LIST   = 5;

const unsigned char ICQ_PLUGIN_REQUEST            = 0;
const unsigned char ICQ_PLUGIN_SUCCESS            = 1;
const unsigned char ICQ_PLUGIN_STATUSxREPLY       = 2;
const unsigned char ICQ_PLUGIN_ERROR              = 3;
const unsigned char ICQ_PLUGIN_REJECTED           = 4;
const unsigned char ICQ_PLUGIN_AWAY               = 5;

const unsigned long ICQ_PLUGIN_STATUSxINACTIVE    = 0;
const unsigned long ICQ_PLUGIN_STATUSxACTIVE      = 1;
const unsigned long ICQ_PLUGIN_STATUSxBUSY        = 2;

/* treat it as response id for now, it might have a different meaning
   judging by the 2 values for info plugin list */
const unsigned long ICQ_PLUGIN_RESP_INFOxLIST   = 0x00010002;
const unsigned long ICQ_PLUGIN_RESP_INFOxLISTx0 = 0x00010000;
const unsigned long ICQ_PLUGIN_RESP_PHONExBOOK  = 0x00000003;
const unsigned long ICQ_PLUGIN_RESP_PICTURE     = 0x00000001;
const unsigned long ICQ_PLUGIN_RESP_STATUSxLIST = 0x00010000;

//charsets
const unsigned short CHARSET_ASCII   = 0x0000;
const unsigned short CHARSET_UNICODE = 0x0002;
const unsigned short CHARSET_CUSTOM  = 0x0003;

// BART Types
const unsigned short BART_TYPExBUDDY_ICON_SMALL = 0x0000;
const unsigned short BART_TYPExBUDDY_ICON       = 0x0001;
const unsigned short BART_TYPExSTATUS_STR       = 0x0002;

//actual plugins
const char PLUGIN_PHONExBOOK[]        = { 0x90, 0x7C, 0x21, 0x2C, 0x91, 0x4D,
                                          0xD3, 0x11, 0xAD, 0xEB, 0x00, 0x04,
                                          0xAC, 0x96, 0xAA, 0xB2, 0x00, 0x00 };

const char PLUGIN_PICTURE[]           = { 0x80, 0x66, 0x28, 0x83, 0x80, 0x28,
                                          0xD3, 0x11, 0x8D, 0xBB, 0x00, 0x10,
                                          0x4B, 0x06, 0x46, 0x2E, 0x00, 0x00 };

const char PLUGIN_FILExSERVER[]       = { 0xF0, 0x2D, 0x12, 0xD9, 0x30, 0x91,
                                          0xD3, 0x11, 0x8D, 0xD7, 0x00, 0x10,
                                          0x4B, 0x06, 0x46, 0x2E, 0x04, 0x00 };

const char PLUGIN_FOLLOWxME[]         = { 0x90, 0x7C, 0x21, 0x2C, 0x91, 0x4D,
                                          0xD3, 0x11, 0xAD, 0xEB, 0x00, 0x04,
                                          0xAC, 0x96, 0xAA, 0xB2, 0x02, 0x00 };

const char PLUGIN_ICQxPHONE[]         = { 0x3F, 0xB6, 0x5E, 0x38, 0xA0, 0x30,
                                          0xD4, 0x11, 0xBD, 0x0F, 0x00, 0x06,
                                          0x29, 0xEE, 0x4D, 0xA1, 0x00, 0x00 };

//these are used to get plugin list
const char PLUGIN_QUERYxINFO[]        = { 0xF0, 0x02, 0xBF, 0x71, 0x43, 0x71,
                                          0xD3, 0x11, 0x8D, 0xD2, 0x00, 0x10,
                                          0x4B, 0x06, 0x46, 0x2E, 0x00, 0x00 };

const char PLUGIN_QUERYxSTATUS[]      = { 0x10, 0x18, 0x06, 0x70, 0x54, 0x71,
                                          0xD3, 0x11, 0x8D, 0xD2, 0x00, 0x10,
                                          0x4B, 0x06, 0x46, 0x2E, 0x00, 0x00 };

// these are sent in handshake packets
const char PLUGIN_INFOxMANAGER[]      = { 0xA0, 0xE9, 0x3F, 0x37, 0x4F, 0xE9,
                                          0xD3, 0x11, 0xBC, 0xD2, 0x00, 0x04,
                                          0xAC, 0x96, 0xDD, 0x96, 0x00, 0x00 };

const char PLUGIN_STATUSxMANAGER[]    = { 0x10, 0xCF, 0x40, 0xD1, 0x4F, 0xE9,
                                          0xD3, 0x11, 0xBC, 0xD2, 0x00, 0x04,
                                          0xAC, 0x96, 0xDD, 0x96, 0x00, 0x00 };

const char PLUGIN_NORMAL[]            = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

//capabilities sent in messages and on login
const char ICQ_CAPABILITY_SRVxRELAY[] = { 0x09, 0x46, 0x13, 0x49, 0x4C, 0x7F,
                                          0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                          0x53, 0x54, 0x00, 0x00 };

const char ICQ_CAPABILITY_DIRECT[]    = { 0x09, 0x46, 0x13, 0x44, 0x4C, 0x7F,
                                          0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                          0x53, 0x54, 0x00, 0x00 };

const char ICQ_CAPABILITY_UTF8[]      = { 0x09, 0x46, 0x13, 0x4E, 0x4C, 0x7F,
                                          0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                          0x53, 0x54, 0x00, 0x00 };

const char ICQ_CAPABILITY_RTFxMSGS[]  = { 0x97, 0xB1, 0x27, 0x51, 0x24, 0x3C,
                                          0x43, 0x34, 0xAD, 0x22, 0xD6, 0xAB,
                                          0xF7, 0x3F, 0x14, 0x92 };

const char ICQ_CAPABILITY_TYPING[]    = { 0x56, 0x3F, 0xC8, 0x09, 0x0B, 0x6F,
                                          0x41, 0xBD, 0x9F, 0x79, 0x42, 0x26,
                                          0x09, 0xDF, 0xA2, 0xF3 };

const char ICQ_CAPABILITY_AIMxINTER[] = { 0x09, 0x46, 0x13, 0x4D, 0x4C, 0x7F,
                                          0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                          0x53, 0x54, 0x00, 0x00 };

const char ICQ_CAPABILITY_ICHAT[]     = { 0x09, 0x46, 0x00, 0x00, 0x4C, 0x7F,
                                          0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                          0x53, 0x54, 0x00, 0x00 };

const char ICQ_CAPABILITY_XTRAZ[]     = { 0x1A, 0x09, 0x3C, 0x6C, 0xD7, 0xFD,
                                          0x4E, 0xC5, 0x9D, 0x51, 0xA6, 0x47,
                                          0x4E, 0x34, 0xF5, 0xA0 };

const char ICQ_CAPABILITY_BART[]      = { 0x09, 0x46, 0x13, 0x46, 0x4C, 0x7F,
                                          0x11, 0xD1, 0x82, 0x22, 0x44, 0x45,
                                          0x53, 0x54, 0x00, 0x00 };

const char ICQ_CAPABILITY_LICQxVER[]  = { 'L',  'i',  'c',  'q',  ' ',  'c',
                                          'l',  'i',  'e',  'n',  't',  ' ',
                                          0x00, 0x00, 0x00, 0x00 };
 
const char ICQ_CAPABILITY_SIMxVER[]   = { 'S',  'I',  'M',  ' ',  'c',  'l', 
                                          'i',  'e',  'n',  't',  ' ',  ' ',
                                          0x00, 0x00, 0x00, 0x00 };

const char ICQ_CAPABILITY_KOPETExVER[] ={ 'K',  'o',  'p',  'e',  't',  'e',
                                          ' ',  'I',  'C',  'Q',  ' ',  ' ',
                                          0x00, 0x00, 0x00, 0x00 };

const char ICQ_CAPABILITY_MICQxVER[]  = { 'm',  'I',  'C',  'Q',  ' ', 0xA9,
                                          ' ',  'R',  '.',  'K',  '.',  ' ', 
                                          0x00, 0x00, 0x00, 0x00 };

// XtraAway
const int XTRA_AWAY_COUNT             = 32;
const char XtraAwayCaps[XTRA_AWAY_COUNT][0x10] = 
{
  { 0x01, 0xD8, 0xD7, 0xEE, 0xAC, 0x3B, 0x49, 0x2A, 0xA5, 0x8D, 0xD3, 0xD8, 0x77, 0xE6, 0x6B, 0x92 },
  { 0x5A, 0x58, 0x1E, 0xA1, 0xE5, 0x80, 0x43, 0x0C, 0xA0, 0x6F, 0x61, 0x22, 0x98, 0xB7, 0xE4, 0xC7 },
  { 0x83, 0xC9, 0xB7, 0x8E, 0x77, 0xE7, 0x43, 0x78, 0xB2, 0xC5, 0xFB, 0x6C, 0xFC, 0xC3, 0x5B, 0xEC },
  { 0xE6, 0x01, 0xE4, 0x1C, 0x33, 0x73, 0x4B, 0xD1, 0xBC, 0x06, 0x81, 0x1D, 0x6C, 0x32, 0x3D, 0x81 },
  { 0x8C, 0x50, 0xDB, 0xAE, 0x81, 0xED, 0x47, 0x86, 0xAC, 0xCA, 0x16, 0xCC, 0x32, 0x13, 0xC7, 0xB7 },
  { 0x3F, 0xB0, 0xBD, 0x36, 0xAF, 0x3B, 0x4A, 0x60, 0x9E, 0xEF, 0xCF, 0x19, 0x0F, 0x6A, 0x5A, 0x7F },
  { 0xF8, 0xE8, 0xD7, 0xB2, 0x82, 0xC4, 0x41, 0x42, 0x90, 0xF8, 0x10, 0xC6, 0xCE, 0x0A, 0x89, 0xA6 },
  { 0x80, 0x53, 0x7D, 0xE2, 0xA4, 0x67, 0x4A, 0x76, 0xB3, 0x54, 0x6D, 0xFD, 0x07, 0x5F, 0x5E, 0xC6 },
  { 0xF1, 0x8A, 0xB5, 0x2E, 0xDC, 0x57, 0x49, 0x1D, 0x99, 0xDC, 0x64, 0x44, 0x50, 0x24, 0x57, 0xAF },
  { 0x1B, 0x78, 0xAE, 0x31, 0xFA, 0x0B, 0x4D, 0x38, 0x93, 0xD1, 0x99, 0x7E, 0xEE, 0xAF, 0xB2, 0x18 },
  { 0x61, 0xBE, 0xE0, 0xDD, 0x8B, 0xDD, 0x47, 0x5D, 0x8D, 0xEE, 0x5F, 0x4B, 0xAA, 0xCF, 0x19, 0xA7 },
  { 0x48, 0x8E, 0x14, 0x89, 0x8A, 0xCA, 0x4A, 0x08, 0x82, 0xAA, 0x77, 0xCE, 0x7A, 0x16, 0x52, 0x08 },
  { 0x10, 0x7A, 0x9A, 0x18, 0x12, 0x32, 0x4D, 0xA4, 0xB6, 0xCD, 0x08, 0x79, 0xDB, 0x78, 0x0F, 0x09 },
  { 0x6F, 0x49, 0x30, 0x98, 0x4F, 0x7C, 0x4A, 0xFF, 0xA2, 0x76, 0x34, 0xA0, 0x3B, 0xCE, 0xAE, 0xA7 },
  { 0x12, 0x92, 0xE5, 0x50, 0x1B, 0x64, 0x4F, 0x66, 0xB2, 0x06, 0xB2, 0x9A, 0xF3, 0x78, 0xE4, 0x8D },
  { 0xD4, 0xA6, 0x11, 0xD0, 0x8F, 0x01, 0x4E, 0xC0, 0x92, 0x23, 0xC5, 0xB6, 0xBE, 0xC6, 0xCC, 0xF0 },
  { 0x60, 0x9D, 0x52, 0xF8, 0xA2, 0x9A, 0x49, 0xA6, 0xB2, 0xA0, 0x25, 0x24, 0xC5, 0xE9, 0xD2, 0x60 },
  { 0x63, 0x62, 0x73, 0x37, 0xA0, 0x3F, 0x49, 0xFF, 0x80, 0xE5, 0xF7, 0x09, 0xCD, 0xE0, 0xA4, 0xEE },
  { 0x1F, 0x7A, 0x40, 0x71, 0xBF, 0x3B, 0x4E, 0x60, 0xBC, 0x32, 0x4C, 0x57, 0x87, 0xB0, 0x4C, 0xF1 },
  { 0x78, 0x5E, 0x8C, 0x48, 0x40, 0xD3, 0x4C, 0x65, 0x88, 0x6F, 0x04, 0xCF, 0x3F, 0x3F, 0x43, 0xDF },
  { 0xA6, 0xED, 0x55, 0x7E, 0x6B, 0xF7, 0x44, 0xD4, 0xA5, 0xD4, 0xD2, 0xE7, 0xD9, 0x5C, 0xE8, 0x1F },
  { 0x12, 0xD0, 0x7E, 0x3E, 0xF8, 0x85, 0x48, 0x9E, 0x8E, 0x97, 0xA7, 0x2A, 0x65, 0x51, 0xE5, 0x8D },
  { 0xBA, 0x74, 0xDB, 0x3E, 0x9E, 0x24, 0x43, 0x4B, 0x87, 0xB6, 0x2F, 0x6B, 0x8D, 0xFE, 0xE5, 0x0F },
  { 0x63, 0x4F, 0x6B, 0xD8, 0xAD, 0xD2, 0x4A, 0xA1, 0xAA, 0xB9, 0x11, 0x5B, 0xC2, 0x6D, 0x05, 0xA1 },
  { 0x2C, 0xE0, 0xE4, 0xE5, 0x7C, 0x64, 0x43, 0x70, 0x9C, 0x3A, 0x7A, 0x1C, 0xE8, 0x78, 0xA7, 0xDC },
  { 0x10, 0x11, 0x17, 0xC9, 0xA3, 0xB0, 0x40, 0xF9, 0x81, 0xAC, 0x49, 0xE1, 0x59, 0xFB, 0xD5, 0xD4 },
  { 0x16, 0x0C, 0x60, 0xBB, 0xDD, 0x44, 0x43, 0xF3, 0x91, 0x40, 0x05, 0x0F, 0x00, 0xE6, 0xC0, 0x09 },
  { 0x64, 0x43, 0xC6, 0xAF, 0x22, 0x60, 0x45, 0x17, 0xB5, 0x8C, 0xD7, 0xDF, 0x8E, 0x29, 0x03, 0x52 },
  { 0x16, 0xF5, 0xB7, 0x6F, 0xA9, 0xD2, 0x40, 0x35, 0x8C, 0xC5, 0xC0, 0x84, 0x70, 0x3C, 0x98, 0xFA },
  { 0x63, 0x14, 0x36, 0xFF, 0x3F, 0x8A, 0x40, 0xD0, 0xA5, 0xCB, 0x7B, 0x66, 0xE0, 0x51, 0xB3, 0x64 },
  { 0xB7, 0x08, 0x67, 0xF5, 0x38, 0x25, 0x43, 0x27, 0xA1, 0xFF, 0xCF, 0x4C, 0xC1, 0x93, 0x97, 0x97 },
  { 0xDD, 0xCF, 0x0E, 0xA9, 0x71, 0x95, 0x40, 0x48, 0xA9, 0xC6, 0x41, 0x32, 0x06, 0xD6, 0xF2, 0x80 }
};

#endif
