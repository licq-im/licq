#ifndef c
#define ORGANIZATION_H

#ifdef __cplusplus
extern "C"
{
#endif

#define NUM_ORGANIZATIONS 20

#ifndef SCategoryDefined
#define SCategoryDefined
struct SCategory
{
  char *szName;          /* Name of the organization */
  unsigned short nCode;  /* Organization code */
  unsigned short nIndex; /* Index in array */
};
#endif
extern const struct SCategory gOrganizations[];

const struct SCategory *GetOrganizationByCode(unsigned short _nCode);
const struct SCategory *GetOrganizationByIndex(unsigned short _nIndex);
const struct SCategory *GetOrganizationByName(const char *_szName);

#ifdef __cplusplus
}
#endif

#endif // ORGANIZATION_H

