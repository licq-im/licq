//
// Many parts of this source code were 'inspired' by the ircII4.4 translat.c source.
// RIPPED FROM KVirc: http://www.kvirc.org
// Original by Szymon Stefanek (kvirc@tin.it).
// Modified by Andrew Frolov (dron@linuxer.net)
// Further modified by Graham Roff

#ifndef _LICQ_TRANSLATE_H_INCLUDED_
#define _LICQ_TRANSLATE_H_INCLUDED_

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
  char *NToRN(const char *_szOldStr);

protected:
        unsigned char serverToClientTab[256];
        unsigned char clientToServerTab[256];
        char *m_szMapFileName, *m_szMapName;
        bool m_bDefault;
};

extern CTranslator gTranslator;

#endif //_LICQ_TRANSLATE_H_INCLUDED_
