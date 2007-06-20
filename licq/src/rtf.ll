%{
    /***************************************************************************
                              rtf.ll  -  description
                                 -------------------
        begin                : Sun Mar 10 2002
        copyright            : (C) 2002 by Vladimir Shutoff
        email                : vovan@shutoff.ru
     ***************************************************************************/

    /***************************************************************************
     *                                                                         *
     *   This program is free software; you can redistribute it and/or modify  *
     *   it under the terms of the GNU General Public License as published by  *
     *   the Free Software Foundation; either version 2 of the License, or     *
     *   (at your option) any later version.                                   *
     *                                                                         *
     ***************************************************************************/

#include <stdio.h>

#include "licq_icqd.h"
#include "licq_translate.h"

#include <vector>
#include <stack>
#include <string>
#include <cstdarg>

#define UP			1	
#define DOWN			2
#define CMD			3
#define TXT			4
#define HEX			5
#define IMG			6
#define UNICODE_CHAR		7
#define SKIP			8
#define SLASH			9
#define S_TXT			10

#define YY_NEVER_INTERACTIVE	1
#define YY_ALWAYS_INTERACTIVE	0
#define YY_MAIN			0	

%}

%option nounput
%option nostack
%option prefix="rtf"

%%

"{"				{ return UP; }
"}"				{ return DOWN; }
"\\"[\\\{\}]			{ return SLASH; }
"\\u"[0-9]{3,7}"?"		{ return UNICODE_CHAR; }
"\\"[A-Za-z]+[0-9]*[ ]? 	{ return CMD; }
"\\'"[0-9A-Fa-f][0-9A-Fa-f]	{ return HEX; }
"<##"[^>]+">"			{ return IMG; }
[^\\{}<]+			{ return TXT; }
.				{ return TXT; }
%%

typedef struct color
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} color;

typedef struct fontDef
{
    int		charset;
    std::string	name;
} fontDef;

class RTF2HTML;

enum Tag
{
    TAG_FONT_SIZE,
    TAG_FONT_COLOR,
    TAG_BG_COLOR,
    TAG_BOLD,
    TAG_ITALIC,
    TAG_UNDERLINE,
    TAG_NONE,
    TAG_ALL
};

class Level
{
public:
    Level(RTF2HTML *_p);
    Level(const Level&);
    void setText(const char* str);
    void setFontTbl() { m_bFontTbl = true; }
    void setColors() { m_bColors = true; resetColors(); }
    void setRed(unsigned char val) { setColor(val, &m_nRed); }
    void setGreen(unsigned char val) { setColor(val, &m_nGreen); }
    void setBlue(unsigned char val) { setColor(val, &m_nBlue); }
    void setFont(unsigned nFont);
    void setEncoding(unsigned nFont);
    void setFontName();
    void setFontColor(unsigned short color);
    void setFontBgColor(unsigned short color);
    void setFontSize(unsigned short size);
    void _setFontSize(unsigned short size);
    void setBold(bool);
    void setItalic(bool);
    void setUnderline(bool);
    void flush();
    void reset();
    void resetTag(Tag);
protected:
    std::string text;
    void Init();
    RTF2HTML *p;
    void resetColors() { m_nRed = m_nGreen = m_nBlue = 0; m_bColorInit = false; }
    void setColor(unsigned char val, unsigned char *p)
    { *p = val; m_bColorInit=true; }
    bool m_bFontTbl;
    bool m_bColors;
    unsigned char m_nRed;
    unsigned char m_nGreen;
    unsigned char m_nBlue;
    bool m_bColorInit;
    unsigned m_nFontColor;
    unsigned m_nFontSize;
    unsigned m_nFontBgColor;
    unsigned m_nTags;
    unsigned m_nEncoding;
    unsigned m_nFontIndex;
    bool m_bFontName;
    bool m_bBold;
    bool m_bItalic;
    bool m_bUnderline;
};

class OutTag
{
public:
    OutTag(Tag _tag, unsigned _param) : tag(_tag), param(_param) {}
    Tag tag;
    unsigned param;
};

class RTF2HTML
{
public:
    RTF2HTML(CICQDaemon *server)
            : rtf_ptr(NULL), m_pDaemon(server), cur_level(this) {}
    std::string Parse(const char *rtf);
    void PrintUnquoted(const char *str, ...);
    void PrintQuoted(const char *str);
protected:
    std::string s;
    const char *rtf_ptr;
    CICQDaemon *m_pDaemon;
    void PutTag(Tag n) { tags.push(n); }
    std::vector<OutTag> oTags;
    std::stack<Tag> tags;
    std::stack<Level> stack;
    Level cur_level;
    std::vector<fontDef> fonts;
    std::vector<color>   colors;
    void FlushOut();
    friend class Level;
};

void RTF2HTML::FlushOut()
{
    std::vector<OutTag>::iterator iter;
    for (iter = oTags.begin(); iter != oTags.end(); iter++)
    {
        OutTag &t = *iter;
        switch (t.tag){
        case TAG_FONT_COLOR:
            if ((unsigned)t.param < (unsigned)colors.size()){
                //color &c = colors[t.param];
                //PrintUnquoted("<font color=\"#%02X%02X%02X\">", c.red, c.green, c.blue);
            }else{
                t.tag = TAG_NONE;
            }
            break;
        case TAG_FONT_SIZE:
            //PrintUnquoted("<font size=%u>", t.param);
            break;
        case TAG_BG_COLOR:{
                //color &c = colors[t.param];
                //PrintUnquoted("<span style=\"bgcolor:#%02X%02X%02X;\">", c.red, c.green, c.blue);
                break;
            }
        case TAG_BOLD:
            //PrintUnquoted("<b>");
            break;
        case TAG_ITALIC:
            //PrintUnquoted("<i>");
            break;
        case TAG_UNDERLINE:
            //PrintUnquoted("<u>");
            break;
        default:
            break;
        }
    }
    oTags.clear();
}

void Level::resetTag(Tag tag)
{
    std::stack<Tag> s;
    while (p->tags.size() > m_nTags){
        Tag nTag = p->tags.top();
        if (p->oTags.empty()){
            switch (nTag){
            case TAG_FONT_COLOR:
            case TAG_FONT_SIZE:
                //p->PrintUnquoted("</font>");
                break;
            case TAG_BG_COLOR:
                //p->PrintUnquoted("</span>");
                break;
            case TAG_BOLD:
                //p->PrintUnquoted("</b>");
                break;
            case TAG_ITALIC:
                //p->PrintUnquoted("</i>");
                break;
            case TAG_UNDERLINE:
                //p->PrintUnquoted("</u>");
                break;
            default:
                break;
            }
        }else{
            p->oTags.pop_back();
        }
        p->tags.pop();
        if (nTag == tag) break;
        s.push(nTag);
    }
    if (tag == 0) return;
    while (!s.empty()){
        Tag nTag = s.top();
        switch (nTag){
        case TAG_FONT_COLOR:{
                unsigned nFontColor = m_nFontColor;
                m_nFontColor = 0;
                setFontColor(nFontColor);
                break;
            }
        case TAG_FONT_SIZE:{
                unsigned nFontSize = m_nFontSize;
                m_nFontSize = 0;
                _setFontSize(nFontSize);
                break;
            }
        case TAG_BG_COLOR:{
                unsigned nFontBgColor = m_nFontBgColor;
                m_nFontBgColor = 0;
                setFontBgColor(nFontBgColor);
                break;
            }
        case TAG_BOLD:{
                bool nBold = m_bBold;
                m_bBold = false;
                setBold(nBold);
                break;
            }
        case TAG_ITALIC:{
                bool nItalic = m_bItalic;
                m_bItalic = false;
                setItalic(nItalic);
                break;
            }
        case TAG_UNDERLINE:{
                bool nUnderline = m_bUnderline;
                m_bUnderline = false;
                setUnderline(nUnderline);
                break;
            }
        default:
            break;
        }
        s.pop();
    }
}

Level::Level(RTF2HTML *_p)
        : p(_p),  m_bFontTbl(false), m_bColors(false),
        m_nEncoding(0), m_nFontIndex((unsigned)(-1))
{
    m_nTags = p->tags.size();
    Init();
}

Level::Level(const Level &l)
        : p(l.p), m_bFontTbl(l.m_bFontTbl), m_bColors(l.m_bColors),
        m_nEncoding(l.m_nEncoding), m_nFontIndex(l.m_nFontIndex)
{
    m_nTags = p->tags.size();
    Init();
}

void Level::Init()
{
    m_nFontColor = 0;
    m_nFontBgColor = 0;
    m_nFontSize = 0;
    m_bFontName = false;
    m_bBold = false;
    m_bItalic = false;
    m_bUnderline = false;
}

void RTF2HTML::PrintUnquoted(const char *str, ...)
{
    char buff[1024];
    va_list ap;
    va_start(ap, str);
    vsnprintf(buff, sizeof(buff), str, ap);
    va_end(ap);
    s += buff;
}

void RTF2HTML::PrintQuoted(const char *_str)
{
    unsigned char *str = (unsigned char*)_str;
    for (; *str; str++){
        unsigned char c = *str;
        if ((c >= 0xC0) && (c <= 0xDF)){
            s += c;
            if ((c = *(++str)) == 0) break;
            s += c;
            continue;
        }
        if ((c >= 0xE0) && (c <= 0xEF)){
            s += c;
            if ((c = *(++str)) == 0) break;
            s += c;
            if ((c = *(++str)) == 0) break;
            s += c;
            continue;
        }
        if ((c >= 0xF0) && (c <= 0xF7)){
            s += c;
            if ((c = *(++str)) == 0) break;
            s += c;
            if ((c = *(++str)) == 0) break;
            s += c;
            if ((c = *(++str)) == 0) break;
            s += c;
            continue;
        }
        if ((c >= 0xF8) && (c <= 0xFB)){
            s += c;
            if ((c = *(++str)) == 0) break;
            s += c;
            if ((c = *(++str)) == 0) break;
            s += c;
            if ((c = *(++str)) == 0) break;
            s += c;
            if ((c = *(++str)) == 0) break;
            s += c;
            continue;
        }
        if ((c >= 0xFC) && (c <= 0xFD)){
            s += c;
            if ((c = *(++str)) == 0) break;
            s += c;
            if ((c = *(++str)) == 0) break;
            s += c;
            if ((c = *(++str)) == 0) break;
            s += c;
            if ((c = *(++str)) == 0) break;
            s += c;
            if ((c = *(++str)) == 0) break;
            s += c;
            continue;
        }
/* This will make the html part, we dont' need it now.
        switch(c){
        case '\n':
            s += "<br>";
            break;
        case '<':
            s += "&lt;";
            break;
        case '>':
            s += "&gt;";
            break;
        case '&':
            s += "&amp;";
            break;
        case '\"':
            s += "&quot;";
            break;
        default:
            if (c >= ' ')
                s += c;
        }*/
        
        s += c;
    }
}

void Level::setFont(unsigned nFont)
{
    if (m_bFontTbl){
        m_nFontIndex = (unsigned)(-1);
        if (nFont > p->fonts.size()){
        }
        if (nFont == p->fonts.size()){
            fontDef f;
            f.charset = 0;
            p->fonts.push_back(f);
        }
        m_nFontIndex = nFont;
        return;
    }
    if (nFont >= p->fonts.size())
        return;
    m_nEncoding = p->fonts[nFont].charset;
}

void Level::setFontName()
{
    if (m_bFontTbl){
        if (m_nFontIndex < p->fonts.size())
            m_bFontName = true;
    }
}

void Level::setEncoding(unsigned nEncoding)
{
    if (m_bFontTbl){
        if (m_nFontIndex < p->fonts.size())
            p->fonts[m_nFontIndex].charset = nEncoding;
        return;
    }
    m_nEncoding = nEncoding;
}

void Level::setBold(bool bBold)
{
    if (m_bBold == bBold) return;
    if (m_bBold) resetTag(TAG_BOLD);
    m_bBold = bBold;
    if (!m_bBold) return;
    p->oTags.push_back(OutTag(TAG_BOLD, 0));
    p->PutTag(TAG_BOLD);
}

void Level::setItalic(bool bItalic)
{
    if (m_bItalic == bItalic) return;
    if (m_bItalic) resetTag(TAG_ITALIC);
    m_bItalic = bItalic;
    if (!m_bItalic) return;
    p->oTags.push_back(OutTag(TAG_ITALIC, 0));
    p->PutTag(TAG_ITALIC);
}

void Level::setUnderline(bool bUnderline)
{
    if (m_bUnderline == bUnderline) return;
    if (m_bUnderline) resetTag(TAG_UNDERLINE);
    m_bUnderline = bUnderline;
    if (!m_bUnderline) return;
    p->oTags.push_back(OutTag(TAG_UNDERLINE, 0));
    p->PutTag(TAG_UNDERLINE);
}

void Level::setFontColor(unsigned short nColor)
{
    if (m_nFontColor == nColor) return;
    if (m_nFontColor) resetTag(TAG_FONT_COLOR);
    m_nFontColor = 0;
    if (nColor == 0) return;
    nColor--;
    if (nColor > p->colors.size()) return;
    p->oTags.push_back(OutTag(TAG_FONT_COLOR, nColor));
    p->PutTag(TAG_FONT_COLOR);
    m_nFontColor = nColor + 1;
}

void Level::setFontBgColor(unsigned short nColor)
{
    if (m_nFontBgColor == nColor) return;
    if (m_nFontBgColor) resetTag(TAG_BG_COLOR);
    m_nFontBgColor = 0;
    if (nColor == 0) return;
    nColor--;
    if (nColor > p->colors.size()) return;
    p->oTags.push_back(OutTag(TAG_BG_COLOR, nColor));
    p->PutTag(TAG_BG_COLOR);
    m_nFontBgColor = nColor + 1;
}

void Level::setFontSize(unsigned short nSize)
{
    if (nSize > 8){
        nSize = (nSize >> 3);
        nSize++;
        if (nSize > 8) nSize = 8;
    }else{
        nSize = 1;
    }
    _setFontSize(nSize);
}

void Level::_setFontSize(unsigned short nSize)
{
    if (m_nFontSize == nSize) return;
    if (m_nFontSize) resetTag(TAG_FONT_SIZE);
    p->oTags.push_back(OutTag(TAG_FONT_SIZE, nSize));
    p->PutTag(TAG_FONT_SIZE);
    m_nFontSize = nSize;
}

void Level::reset()
{
    resetTag(TAG_ALL);
    if (m_bColors){
        if (m_bColorInit){
            color c;
            c.red = m_nRed;
            c.green = m_nGreen;
            c.blue = m_nBlue;
            p->colors.push_back(c);
            resetColors();
        }
        return;
    }
}

void Level::setText(const char *str)
{
    if (m_bColors){
        reset();
        return;
    }
    if (m_bFontName){
        char *pp = strchr(str, ';');
        unsigned size = strlen(pp);
        if (pp){
            size = (pp - str);
            m_bFontName = false;
        }
        if (m_nFontIndex < p->fonts.size())
            p->fonts[m_nFontIndex].name.append(str, size);
        return;
    }
    if (m_bFontTbl) return;
    for (; *str; str++)
        if ((unsigned char)(*str) >= ' ') break;
    if (!*str) return;
    p->FlushOut();
    text += str;
}

void Level::flush()
{
    if (text.length() == 0) return;
/* We are using encodings per user, so skip this part*/
    /*const char *encoding = NULL;
    if (m_nEncoding){
        for (const rtf_charset *c = rtf_charsets; c->rtf_code; c++){
            if ((unsigned)c->rtf_code == m_nEncoding){
                encoding = c->name;
                break;
            }
        }
    }
    if (encoding == NULL) encoding = p->encoding;
    p->icq->client->toUTF(text, encoding);*/
    /*char *szText = gTranslator.ToUnicode(text);
    text = szText;*/
    char *szText = const_cast<char *>(text.c_str());
    gTranslator.ServerToClient(szText);
    text = szText;
    p->PrintQuoted(text.c_str());
    text = "";
}

const unsigned FONTTBL		= 0;
const unsigned COLORTBL		= 1;
const unsigned RED			= 2;
const unsigned GREEN		= 3;
const unsigned BLUE			= 4;
const unsigned CF			= 5;
const unsigned FSX			= 6;
const unsigned HIGHLIGHT	= 7;
const unsigned PARD			= 8;
const unsigned PAR			= 9;
const unsigned I			= 10;
const unsigned B			= 11;
const unsigned UL			= 12;
const unsigned F			= 13;
const unsigned FCHARSET		= 14;
const unsigned FNAME		= 15;
const unsigned ULNONE		= 16;

static char cmds[] =
    "fonttbl\x00"
    "colortbl\x00"
    "red\x00"
    "green\x00"
    "blue\x00"
    "cf\x00"
    "fs\x00"
    "highlight\x00"
    "pard\x00"
    "par\x00"
    "i\x00"
    "b\x00"
    "ul\x00"
    "f\x00"
    "fcharset\x00"
    "fname\x00"
    "ulnone\x00"
    "\x00";

int yywrap() { return 1; }

static char h2d(char c)
{
    if ((c >= '0') && (c <= '9'))
        return c - '0';
    if ((c >= 'A') && (c <= 'F'))
        return (c - 'A') + 10;
    if ((c >= 'a') && (c <= 'f'))
        return (c - 'a') + 10;
    return 0;
}

std::string RTF2HTML::Parse(const char *rtf)
{
    yy_current_buffer = yy_scan_string(rtf);
    s.erase();
    rtf_ptr = rtf;
    for (;;){
        int res = yylex();
        if (!res) break;
        switch (res){
        case UP:{
                cur_level.flush();
                stack.push(cur_level);
                break;
            }
        case DOWN:{
				if (stack.size()){
                cur_level.flush();
                cur_level.reset();
                cur_level = stack.top();
                stack.pop();
				}
                break;
            }
        case IMG:{
                cur_level.flush();
                const char ICQIMAGE[] = "icqimage";
                const char *p = yytext + 3;
                if ((strlen(p) > strlen(ICQIMAGE)) && !memcmp(p, ICQIMAGE, strlen(ICQIMAGE))){
                    unsigned n = 0;
                    for (p += strlen(ICQIMAGE); *p; p++){
                        if ((*p >= '0') && (*p <= '9')){
                            n = n << 4;
                            n += (*p - '0');
                            continue;
                        }
                        if ((*p >= 'A') && (*p <= 'F')){
                            n = n << 4;
                            n += (*p - 'A') + 10;
                            continue;
                        }
                        if ((*p >= 'a') && (*p <= 'f')){
                            n = n << 4;
                            n += (*p - 'a') + 10;
                            continue;
                        }
                        break;
                    }
                    switch (n){
                    case 0x00:
                        PrintQuoted(":-)");
                        break;
                    case 0x01:
                        PrintQuoted(":-O");
                        break;
                    case 0x02:
                        PrintQuoted(":-|");
                        break;
                    case 0x03:
                        PrintQuoted(":-/");
                        break;
                    case 0x04:
                        PrintQuoted(":-(");
                        break;
                    case 0x05:
                        PrintQuoted(":-{}");
                        break;
                    case 0x06:
                        PrintQuoted(":*)");
                        break;
                    case 0x07:
                        PrintQuoted(":'-(");
                        break;
                    case 0x08:
                        PrintQuoted(";-)");
                        break;
                    case 0x09:
                        PrintQuoted(":-@");
                        break;
                    case 0x0A:
                        PrintQuoted(":-");
                        break;
                    case 0x0B:
                        PrintQuoted(":-X");
                        break;
                    case 0x0C:
                        PrintQuoted(":-P");
                        break;
                    case 0x0D:
                        PrintQuoted("8-)");
                        break;
                    case 0x0E:
                        PrintQuoted("O-)");
                        break;
                    case 0x0F:
                        PrintQuoted(":-D");
                        break;
                    case 0x10:
                        PrintQuoted("*STOP*");
                        break;
                    case 0x14:
                        PrintQuoted("*JOKINGLY*");
                        break;
                    }
                }else{
                }
                break;
            }
        case SKIP:
            break;
        case SLASH:
            cur_level.setText(yytext+1);
            break;
        case TXT:
            cur_level.setText(yytext);
            break;
        case UNICODE_CHAR:{
                cur_level.flush();
                std::string s;
                gTranslator.utf16to8(atol(yytext + 2), s);
                PrintQuoted(s.c_str());
                break;
            }
        case HEX:{
                cur_level.flush();
                char s[2];
                s[0] = (h2d(yytext[2]) << 4) + h2d(yytext[3]);
                s[1] = 0;
                cur_level.setText(s);
                break;
            }
        case CMD:
            {
                cur_level.flush();
                const char *cmd = yytext + 1;
                unsigned n_cmd = 0;
                unsigned cmd_size = 0;
                int cmd_value = -1;
                const char *p;
                for (p = cmd; *p; p++, cmd_size++)
                    if (((*p >= '0') && (*p <= '9')) || (*p == ' ')) break;
                if (*p && (*p != ' ')) cmd_value = atol(p);
                for (p = cmds; *p; p += strlen(p) + 1, n_cmd++){
                    if (strlen(p) >  cmd_size) continue;
                    if (!memcmp(p, cmd, cmd_size)) break;
                }
                cmd += strlen(p);
                switch (n_cmd){
                case FONTTBL:		// fonttbl
                    cur_level.setFontTbl();
                    break;
                case COLORTBL:
                    cur_level.setColors();
                    break;
                case RED:
                    cur_level.setRed(cmd_value);
                    break;
                case GREEN:
                    cur_level.setGreen(cmd_value);
                    break;
                case BLUE:
                    cur_level.setBlue(cmd_value);
                    break;
                case CF:
                    cur_level.setFontColor(cmd_value);
                    break;
                case FSX:
                    cur_level.setFontSize(cmd_value);
                    break;
                case HIGHLIGHT:
                    cur_level.setFontBgColor(cmd_value);
                    break;
                case PARD:
                    break;
                case PAR:
                    PrintUnquoted("\n");
                    break;
                case I:
                    cur_level.setItalic(cmd_value != 0);
                    break;
                case B:
                    cur_level.setBold(cmd_value != 0);
                    break;
                case UL:
                    cur_level.setUnderline(cmd_value != 0);
                    break;
                case ULNONE:
                    cur_level.setUnderline(false);
                    break;
                case F:
                    cur_level.setFont(cmd_value);
                    break;
                case FCHARSET:
                    cur_level.setEncoding(cmd_value);
                    break;
                case FNAME:
                    cur_level.setFontName();
                    break;
                }
                break;
            }
        }
    }
    yy_delete_buffer(yy_current_buffer);
    yy_current_buffer = NULL;
    return s;
}

char *CICQDaemon::parseRTF(const char *rtf)
{
    std::string str;
    char _RTF[] = "{\\rtf";
    if ((strlen(rtf) > strlen(_RTF)) && !memcmp(rtf, _RTF, strlen(_RTF)))
    {
      RTF2HTML p(this);
      str = p.Parse(rtf);
    }
    else
      str = rtf;

    char *szReturn = new char[str.length() + 1];
    strcpy(szReturn, str.c_str());
    szReturn[str.length()] = '\0';
    return szReturn;
}

