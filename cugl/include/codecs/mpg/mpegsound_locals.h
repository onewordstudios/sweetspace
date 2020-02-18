/* MPEG/WAVE Sound library

   (C) 1997 by Jung woo-jae */

// Mpegsound_locals.h
// It is used for compiling library

#ifndef _L__SOUND_LOCALS__
#define _L__SOUND_LOCALS__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Inline functions
inline int Mpegtoraw::getbyte(void)
{
  int r = (unsigned char)buffer[ bitindex >> 3 ];
  bitindex+=8;
  return r;
};

inline int Mpegtoraw::getbits9(int bits)
{
  unsigned short a;
  int offset=bitindex>>3;
#ifndef WORDS_BIGENDIAN
  {
    a=(((unsigned char)buffer[offset])<<8) | ((unsigned char)buffer[offset+1]);
  }
#else
  a=(((unsigned char)buffer[offset+1])<<8) | ((unsigned char)buffer[offset]);
  /* This will not work. It can cause Bus Errors in short alignment is not correct*/
  /* a=*((unsigned short *)(buffer+((bitindex>>3)))); */
#endif

  a<<=(bitindex&7);
  bitindex+=bits;
  return (int)((unsigned int)(a>>(16-bits)));
};

inline int Mpegtoraw::getbits8(void)
{
  unsigned short a;
  int offset=bitindex>>3;

  a=(((unsigned char)buffer[offset])<<8) | ((unsigned char)buffer[offset+1]);

  a<<=(bitindex&7);
  bitindex+=8;
  return (int)((unsigned int)(a>>8));
};

inline int Mpegtoraw::getbit(void)
{
  int r=(buffer[bitindex>>3]>>(7-(bitindex&7)))&1;

  bitindex++;
  return r;
};

#endif
