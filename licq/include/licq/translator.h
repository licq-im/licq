//
// Many parts of this source code were 'inspired' by the ircII4.4 translat.c source.
// RIPPED FROM KVirc: http://www.kvirc.org
// Original by Szymon Stefanek (kvirc@tin.it).
// Modified by Andrew Frolov (dron@linuxer.net)
// Further modified by Graham Roff

#ifndef LICQ_TRANSLATOR_H
#define LICQ_TRANSLATOR_H

#include <string>

namespace Licq
{

class Translator
{
public:
  Translator();
  ~Translator();

  void setDefaultTranslationMap();
  bool setTranslationMap(const std::string& mapFileName);
  bool isDefaultMap() { return myMapDefault; }
  const std::string& getMapName() const { return myMapName; }
  const std::string& getMapFileName() const { return myMapFileName; }

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
  std::string myMapName;
  std::string myMapFileName;
  unsigned char serverToClientTab[256];
  unsigned char clientToServerTab[256];

  char* iconvConvert(const char* array, const char* to, const char* from,
      bool& ok, int length = -1, size_t* outDone = NULL);
};

extern Translator gTranslator;

} // namespace Licq

#endif
