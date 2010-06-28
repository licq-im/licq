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

  std::string serverToClient(const std::string& s);
  char serverToClient(char c);
  std::string clientToServer(const std::string& s);
  char clientToServer(char c);

  bool isAscii(const std::string& s);

  std::string fromUnicode(const std::string& s, const std::string& toEncoding = "");
  std::string toUnicode(const std::string& s, const std::string& fromEncoding = "");
  std::string fromUtf16(const std::string& s, const std::string& toEncoding);
  std::string toUtf16(const std::string& s, const std::string& fromEncoding);

  /**
   * Converts a unix style string (LF) to dos style (LFCR)
   */
  std::string returnToDos(const std::string& s);

  /**
   * Converts a dos (CRLF) or mac style (CR) style string to unix style (LF)
   */
  std::string returnToUnix(const std::string& s);

  void ServerToClient(char* array);
  void ServerToClient(char& value);
  void ClientToServer(char* array);
  void ClientToServer(char& value);

  // Must use delete[] on the returned value if it is not NULL

  char* ToUnicode(const char* array, const char* fromEncoding = "");
  char* FromUnicode(const char* array, const char* toEncoding = "");

  char* FromUTF16(const char* array, const char* toEncoding, int length = -1);
  char* ToUTF16(const char* array, const char* fromEncoding, size_t& outDone);

  char* NToRN(const char* array);
  char* RNToN(const char* array);

  bool utf16to8(unsigned long c, std::string &s);

protected:
  std::string nameForIconv(const std::string& licqName);

  std::string iconvConvert(const std::string& s, const std::string& to, const std::string& from,
      bool& ok, int length = -1, size_t* outDone = NULL);

  bool myMapDefault;
  std::string myMapName;
  std::string myMapFileName;
  unsigned char serverToClientTab[256];
  unsigned char clientToServerTab[256];
};

extern Translator gTranslator;

} // namespace Licq

#endif
