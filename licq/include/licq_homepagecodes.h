#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#ifdef __cplusplus
extern "C"
{
#endif

#define NUM_HOMEPAGECATS 2985

struct SHomepageCat
{
  char *szName;          /* Name of the category */
  unsigned short nCode;  /* Category code */
  unsigned short nIndex; /* Index in array */
};
extern const struct SHomepageCat gHomepageCategories[];

const struct SHomepageCat *GetHomepageCatByCode(unsigned short _nCode);
const struct SHomepageCat *GetHomepageCatByIndex(unsigned short _nIndex);
const struct SHomepageCat *GetHomepageCatByName(const char *_szName);

#ifdef __cplusplus
}
#endif

#endif // HOMEPAGE_H

