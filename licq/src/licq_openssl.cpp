#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "licq_openssl.h"

#ifdef USE_OPENSSL

#include <stdio.h>
#include <openssl/crypto.h>
#include <openssl/des.h>
#include <openssl/bn.h>
#include <string.h>

#include "licq_buffer.h"
#include "licq_log.h"


CDHKey::CDHKey()
{
  m_p = BN_new();
  m_a = BN_new();
  m_g = BN_new();
  m_gab = BN_new();
  m_nCryptoStatus = CRYPTO_NONE;
}


CBuffer *CDHKey::DesXEncrypt(CBuffer *inbuf)
{
  if (inbuf->getDataSize() == 0) return NULL;

  int data_size = inbuf->getDataSize() + 1;
  // Align on 8 byte boundary
  if (data_size & 0x07)
  {
    data_size >>= 3;
    data_size <<= 3;
    data_size += 8;
  }

  // Prepare in buffer...sucks having to make an extra copy...
  unsigned char *data_in = (unsigned char *)malloc(data_size);
  memset(data_in, 0, data_size);
  memcpy(data_in, inbuf->getDataStart(), inbuf->getDataSize());
  data_in[data_size - 1] = data_size - inbuf->getDataSize();

  // Prepare out buffer
  CBuffer *outbuf = new CBuffer(data_size);
  unsigned char *data_out = (unsigned char *)outbuf->getDataStart();
  memset(data_out, 0, data_size);

  des_cblock iv;
  des_key_schedule ks;
  unsigned char wholebinkey[DES_KEY_SZ * 3];
  des_cblock binkey;
  des_cblock xin;
  des_cblock xout;

  memset(wholebinkey, 0, DES_KEY_SZ * 3);
  BN_bn2bin(m_gab, wholebinkey);

  memset(binkey, 0, DES_KEY_SZ);
  memset(xin, 0, DES_KEY_SZ);
  memset(xout, 0, DES_KEY_SZ);

  memcpy(binkey, wholebinkey, DES_KEY_SZ); // the key
  memcpy(xin, wholebinkey + DES_KEY_SZ, DES_KEY_SZ); // in white
  memcpy(xout, wholebinkey + DES_KEY_SZ * 2, DES_KEY_SZ); // out white
  des_set_odd_parity(&binkey);
  des_set_odd_parity(&xin);
  des_set_odd_parity(&xout);
  memcpy(iv, xout, DES_KEY_SZ); // iv (cbc)

  int j;
  if ((j = des_set_key_checked(&binkey, ks)) != 0)
  {
    gLog.Warn("%sError checking encryption key: %d\n", L_DESxSTR, j);
    free(data_in);
    return NULL;
  }

  // Encrypt it
  des_xcbc_encrypt(data_in, data_out, data_size, ks, &iv, &xin,
     &xout, DES_ENCRYPT);

  // Go to the end of the buffer now
  outbuf->setDataPosWrite(outbuf->getDataStart() + data_size);

  // Clean up
  free(data_in);

  return outbuf;
}


CBuffer *CDHKey::DesXDecrypt(CBuffer *inbuf)
{
  unsigned char *data_in = (unsigned char *)inbuf->getDataStart();

  if (data_in == NULL) return NULL;

  int data_size = inbuf->getDataSize();
  CBuffer *outbuf = new CBuffer(data_size + 8);
  unsigned char *data_out = (unsigned char *)outbuf->getDataStart();
  memset(data_out, 0, outbuf->getDataSize());

  // Hope that the data is 8-byte aligned...we save enough room in the
  // output buffer, but the input buffer will be overflow read and
  // the decryption will be wrong in the last bytes
  if (data_size & 0x07)
  {
    gLog.Warn("%sDesXDecrypt: data not 8-byte aligned.\n", L_WARNxSTR);
  }

  des_cblock iv;
  des_key_schedule ks;
  unsigned char wholebinkey[DES_KEY_SZ * 3];
  des_cblock binkey;
  des_cblock xin;
  des_cblock xout;

  memset(wholebinkey, 0, DES_KEY_SZ * 3);
  /*int t =*/ BN_bn2bin(m_gab, wholebinkey);

  //char *tt = BN_bn2hex(m_gab);
  //fprintf(stderr, "Key size: %d bytes\nkey: [%s]\n\n", t, tt);

  memset(binkey, 0, DES_KEY_SZ);
  memset(xin, 0, DES_KEY_SZ);
  memset(xout, 0, DES_KEY_SZ);

  memcpy(binkey, wholebinkey, DES_KEY_SZ); // the key
  memcpy(xin, wholebinkey + DES_KEY_SZ,  DES_KEY_SZ); // in white
  memcpy(xout, wholebinkey + DES_KEY_SZ * 2,  DES_KEY_SZ); // out white
  des_set_odd_parity(&binkey);
  des_set_odd_parity(&xin);
  des_set_odd_parity(&xout);
  memcpy(iv, xout, DES_KEY_SZ); // iv (cbc)

  int j;
  if ((j = des_set_key_checked(&binkey, ks)) != 0)
  {
    gLog.Warn("%sError checking decryption key: %d\n", L_DESxSTR, j);
    return NULL;
  }

  des_xcbc_encrypt(data_in, data_out, data_size, ks, &iv, &xin,
   &xout, DES_DECRYPT);

  // Go to the end of the buffer now
  outbuf->setDataPosWrite(outbuf->getDataStart()
   + data_size
   - outbuf->getDataStart()[data_size - 1]);

  return outbuf;
}

#if 0

char *CDHKey::DesXEncrypt_String(unsigned char *data)
{
  int j;
  des_cblock iv;
  des_m_gab_schedule ks;
  unsigned char padded_data[385] = {32};
  char *final_data = NULL;

  if (data == NULL) return (NULL);

  padded_data[384] = (unsigned char) 0;

  j = 0;
  while ((data[j] != 0) && (j < 385))
  {
    padded_data[j] = data[j];
    j++;
  }

  int data_size = sizeof(unsigned char) * (strlen(padded_data) + 1);
  unsigned char *data_out = (unsigned char *) malloc (385);
  unsigned char wholebinkey[24];
  des_cblock binkey[8];
  des_cblock xin [8];
  des_cblock xout[8];
  char size[24] = {0};
  int sizesize;

  sprintf(size, "%d\n", /* data_size-1 */ 384);
  sizesize = strlen(size);

  memset(wholebinkey, 0, 24);
  int t = BN_bn2bin(m_gab, wholebinkey);
  char *tt = BN_bn2hex(m_gab);

  fprintf(stderr, "Key size: %d bytes\nm_gab: [%s]\nData: \n[%s]\n--\n", t, tt, padded_data);	
  fprintf(stderr, "size: %s bytes\nsizesize: %d\n", size, sizesize);

  memset(binkey, 0, 8);
  memset(xin, 0, 8);
  memset(xout, 0, 8);

  memcpy(binkey, wholebinkey,       8); // the key
  memcpy(xin,    wholebinkey +  8,  8); // in white
  memcpy(xout,   wholebinkey + 16,  8); // out white
  //  memcpy(iv,     wholebinkey + 16,  8); // iv (cbc)
  des_set_odd_parity(binkey);
  des_set_odd_parity(xin);
  des_set_odd_parity(xout);
  memcpy(iv, xout, 8); // iv (cbc)

  printf("------------\nDoing desx.cbc\n");
  if ((j=des_set_m_gab_checked(binkey, ks)) != 0){
	  printf("ERR: %d\n",j);
  }
  memset(data_out, 0, data_size);
  des_xcbc_encrypt(padded_data, data_out, 384, ks, &iv, xin,xout, DES_ENCRYPT);

  final_data = (char *) malloc (sizeof (data_out) + sizeof (sizesize) + 3);
  for (j = 0; j < (sizeof(data_out) + sizeof(sizesize) + 3); j++)
  {
    final_data[j] = 0;
  }

  fprintf (stderr, "[CRPT] final size: %i\n", sizeof (data_out) + sizeof (sizesize) + 3);

  //sprintf (final_data, "%s%s", size, data_out);

  memcpy (final_data, size, sizesize);
  memcpy (final_data + sizesize, data_out, 384);

  return (final_data);
}



void CDHKey::DesXDecrypt_String(unsigned char *data)
{
  int j;
  des_cblock iv;
  des_key_schedule ks;
  char size[24] = {0};
  int index = 0;
  unsigned char *m_data = data;

  if(data == NULL) return;

  for(index = 0;m_data[index] != '\n'; index++)
  {
    size[index] = m_data[index];
  }
  size[index] = (char) 0;
  m_data = data + index + 1;

  // fprintf(stderr, "m_pre: [%s]\n", m_data);

  int data_size = atoi(size) /* + 1 */;

  fprintf(stderr, "Data segment size: %d bytes\n", data_size);

  unsigned char * data_out = (unsigned char *) malloc (data_size);
  unsigned char wholebinkey[24];
  des_cblock binkey[8];
  des_cblock xin [8];
  des_cblock xout[8];

  memset(wholebinkey, 0, 24);
  int t = BN_bn2bin(m_gab, wholebinkey);
  char * tt = BN_bn2hex(m_gab);

  fprintf(stderr, "Key size: %d bytes\nkey: [%s]\n\n", t, tt);

  memset(binkey, 0, 8);
  memset(xin, 0, 8);
  memset(xout, 0, 8);

  memcpy(binkey, wholebinkey,       8); // the key
  memcpy(xin,    wholebinkey +  8,  8); // in white
  memcpy(xout,   wholebinkey + 16,  8); // out white
  //  memcpy(iv,     wholebinkey + 16,  8); // iv (cbc)
  des_set_odd_parity(binkey);
  des_set_odd_parity(xin);
  des_set_odd_parity(xout);
  memcpy(iv, xout, 8); // iv (cbc)

  printf("------------\nDoing desx.cbc\n");
  if ((j=des_set_key_checked(binkey, ks)) != 0)
  {
	  printf("ERR: %d\n",j);
  }
  // printf("pre:\n[%s]\n--\n", data);
  memset(data_out, 0, data_size);
  des_xcbc_encrypt(m_data, data_out, data_size, ks, &iv, xin, xout, DES_DECRYPT);

  printf("de:\n[%s]\n--\n", data_out);
  // sprintf (data, "%s", data_out);

  memcpy(data, data_out, data_size);
  *(data + data_size) = (unsigned char) 0;
  return;
}
#endif

#endif
