#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"

#include <stdio.h>
#include <string.h>

#include "licq_providers.h"

const struct SProvider gProviders[NUM_PROVIDERS] =
{
  { tr_NOOP("(Brazil) Access"), tr_NOOP("@email.pageacess.com.br"), 0 },
  { tr_NOOP("(Brazil) E-Trim"), tr_NOOP("@etrim.com.br"), 1 },
  { tr_NOOP("(Canada) BC Tel Mobility"), tr_NOOP("@message.bctm.com"), 2 },
  { tr_NOOP("(Canada) Select Communication"), tr_NOOP("@selectcomm.ab.ca"), 3 },
  { tr_NOOP("(Canada) TeleLink"), tr_NOOP("@ds5k.telelink.com"), 4 },
  { tr_NOOP("(Canada) Bell Mobility"), tr_NOOP("@pager.mobility.com"), 5 },
  { tr_NOOP("(India) MessageIndia"), tr_NOOP("@messageindia.com"), 6 },
  { tr_NOOP("(India) MobiLink"), tr_NOOP("@MobilinkIndia.com"), 7 },
  { tr_NOOP("(India) PageLink"), tr_NOOP("@pagelink-india.com"), 8 },
  { tr_NOOP("(Malaysia) EasyCall"), tr_NOOP("@easycall.com.my"), 9 },
  { tr_NOOP("(Mexico) WebLink Wireless (PageMart) (Traditional)"), tr_NOOP("@pagemart.net"), 10 },
  { tr_NOOP("(Mexico) WebLink Wireless (PageMart) (Advanced)"), tr_NOOP("@airmessage.net"), 11 },
  { tr_NOOP("(Russia) F1"), tr_NOOP("@pager.oryol.ru"), 12 },
  { tr_NOOP("(Russia) FCN (Full Cry Neva)"), tr_NOOP("@pager.fcn.ru"), 13 },
  { tr_NOOP("(Russia) Multi-Page"), tr_NOOP("@pager.multi-page.ru"), 14 },
  { tr_NOOP("(Russia) Mobile Telecom"), tr_NOOP("@pager.mtelecom.ru"), 15 },
  { tr_NOOP("(Russia) Mobile Telecom (subject)"), tr_NOOP("@notify.mtelecom.ru"), 16 },
  { tr_NOOP("(Singapore) Hutchinson Paging SG"), tr_NOOP("@hutch.com.sg"), 17 },
  { tr_NOOP("(Singapore) M1"), tr_NOOP("@m1.com.sg"), 18 },
  { tr_NOOP("(Ukraine) U-Page (Eurotext)"), tr_NOOP("@eurotext.kiev.ua"), 19 },
  { tr_NOOP("(USA) Ameritech/SBC"), tr_NOOP("@paging.acswireless.com"), 20 },
  { tr_NOOP("(USA) Arch"), tr_NOOP("@epage.arch.com"), 21 },
  { tr_NOOP("(USA) AT&T"), tr_NOOP("@mobile.att.net"), 22 },
  { tr_NOOP("(USA) Infinitel"), tr_NOOP("@alphame.com"), 23 },
  { tr_NOOP("(USA) Metrocall"), tr_NOOP("@page.metrocall.com"), 24 },
  { tr_NOOP("(USA) PageNet"), tr_NOOP("@pagenet.net"), 25 },
  { tr_NOOP("(USA) ProComWireless"), tr_NOOP("@page.procomwireless.com"), 26 },
  { tr_NOOP("(USA) SkyTel/MCI"), tr_NOOP("@skytel.com"), 27 },
  { tr_NOOP("(USA) TeleBeep"), tr_NOOP("@page.pnet5.com"), 28 },
  { tr_NOOP("(USA) TeleTouch"), tr_NOOP("@pageme.teletouch.com"), 29 },
  { tr_NOOP("(USA) WebLink Wireless (PageMart) (Traditional)"), tr_NOOP("@pagemart.net"), 30 },
  { tr_NOOP("(USA) WebLink Wireless (PageMart) (Advanced)"), tr_NOOP("@airmessage.net"), 31 }
};


const struct SProvider *GetProviderByGateway(const char *_szGateway)
{
   unsigned short i = 0;
   while (i < NUM_PROVIDERS && strcasecmp(gProviders[i].szGateway, _szGateway))
     i++;
   if (i == NUM_PROVIDERS) return NULL;
   return &gProviders[i];
}

const struct SProvider *GetProviderByIndex(unsigned short _nIndex)
{
   if (_nIndex >= NUM_PROVIDERS) return NULL;
   return (&gProviders[_nIndex]);
}

const struct SProvider *GetProviderByName(const char *_szName)
{
   unsigned short i = 0;
   while (i < NUM_PROVIDERS && strcasecmp(gProviders[i].szName, _szName)) i++;
   if (i == NUM_PROVIDERS) return NULL;
   return &gProviders[i];
}
