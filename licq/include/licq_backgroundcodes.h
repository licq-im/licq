#ifndef BACKGROUND_H
#define BACKGROUND_H

#ifdef __cplusplus
extern "C"
{
#endif

#define NUM_BACKGROUNDS 8

#ifndef SCategoryDefined
#define SCategoryDefined
struct SCategory
{
  char *szName;          /* Name of the background */
  unsigned short nCode;  /* Background code */
  unsigned short nIndex; /* Index in array */
};
#endif
extern const struct SCategory gBackgrounds[];

const struct SCategory *GetBackgroundByCode(unsigned short _nCode);
const struct SCategory *GetBackgroundByIndex(unsigned short _nIndex);
const struct SCategory *GetBackgroundByName(const char *_szName);

#ifdef __cplusplus
}
#endif

#endif // BACKGROUND_H

