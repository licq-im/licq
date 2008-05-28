//
// Many parts of this source code were 'inspired' by the ircII4.4 translat.c source.
// RIPPED FROM KVirc: http://www.kvirc.org
// Original by Szymon Stefanek (kvirc@tin.it).
// Modified by Andrew Frolov (dron@linuxer.net)
// Further modified by Graham Roff

#ifndef _LICQ_TRANSLATE_H_INCLUDED_
#define _LICQ_TRANSLATE_H_INCLUDED_

#include <string>


class CTranslator
{
public:
  CTranslator();
  ~CTranslator();

  void setDefaultTranslationMap();
  bool setTranslationMap(const char* mapFileName);
  bool isDefaultMap() { return myMapDefault; }
  const char* getMapName() { return myMapName; }
  const char* getMapFileName() { return myMapFileName; }

  void ServerToClient(char* array);
  void ServerToClient(char& value);
  void ClientToServer(char* array);
  void ClientToServer(char& value);

  bool isAscii(const char* array, int length = -1);

  // Must use delete[] on the returned value if it is not NULL
  char* nameForIconv(const char* licqName);

  char* ToUnicode(const char* array, const char* fromEncoding = "");
  char* FromUnicode(const char* array, const char* toEncoding = "");

  char* FromUTF16(const char* array, const char* toEncoding, int length = -1);
  char* ToUTF16(const char* array, const char* fromEncoding, size_t& outDone);

  char* NToRN(const char* array);
  char* RNToN(const char* array);

  bool utf16to8(unsigned long c, std::string &s);

protected:
  bool myMapDefault;
  char* myMapName;
  char* myMapFileName;
  unsigned char serverToClientTab[256];
  unsigned char clientToServerTab[256];

  char* iconvConvert(const char* array, const char* to, const char* from,
      bool& ok, int length = -1, size_t* outDone = NULL);
};

extern CTranslator gTranslator;

#endif //_LICQ_TRANSLATE_H_INCLUDED_
