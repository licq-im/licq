#ifndef PROVIDERS_H
#define PROVIDERS_H

#ifdef __cplusplus
extern "C"
{
#endif

#define NUM_PROVIDERS 32

struct SProvider
{
  char *szName;          /* Name of provider */
  char *szGateway;       /* E-mail gateway of the provider */
  unsigned short nIndex; /* Index in array */
};
extern const struct SProvider gProviders[];

const struct SProvider *GetProviderByGateway(const char *_szGateway);
const struct SProvider *GetProviderByIndex(unsigned short _nIndex);
const struct SProvider *GetProviderByName(const char *_szName);

#ifdef __cplusplus
}
#endif

#endif // PROVIDERS_H
