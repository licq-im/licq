//
// Many parts of this source code were 'inspired' by the ircII4.4 translat.c source.
// RIPPED FROM KVirc: http://www.kvirc.org
// Original by Szymon Stefanek (kvirc@tin.it).
// Modified by Andrew Frolov (dron@linuxer.net)
// Further modified by Graham Roff

#ifndef _LICQ_TRANSLATE_H_INCLUDED_
#define _LICQ_TRANSLATE_H_INCLUDED_

#include <string>

using namespace std;

class CTranslator
{
public:
    CTranslator();
    ~CTranslator();
    void setDefaultTranslationMap();
    bool setTranslationMap(const char *szMapFileName);
    void ServerToClient(char *szString);
    void ClientToServer(char *szString);
    void ServerToClient(char &_cChar);
    void ClientToServer(char &_cChar);
    bool usingDefaultMap()  { return m_bDefault; }
    const char *getMapFileName() { return m_szMapFileName; }
    const char *getMapName() { return m_szMapName; }
    
    unsigned short CheckEncoding(const char *, int);
    
    // Muse use delete[] on the returned value if it is not NULL
  char* ToUnicode(char* _sz, const char *_szFrom = "");
  char* FromUnicode(char* _sz, const char* _szTo = "");
    char *FromUTF16(char *_sz, int nMsgLen = -1);
    char *ToUTF16(char *_sz, char *_szEncoding, size_t &nLen);
    char *NToRN(const char* _szOldStr);
    char *RNToN(const char* _szOldStr);
    bool utf16to8(unsigned long c, string &s);

protected:
    unsigned char serverToClientTab[256];
    unsigned char clientToServerTab[256];
    char *m_szMapFileName, *m_szMapName;
    bool m_bDefault;
};

extern CTranslator gTranslator;

#endif //_LICQ_TRANSLATE_H_INCLUDED_
