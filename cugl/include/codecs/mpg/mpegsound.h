/* MPEG/WAVE Sound library

   (C) 1997 by Woo-jae Jung */

// Mpegsound.h
//   This is typeset for functions in MPEG/WAVE Sound library.
//   Now, it's for only linux-pc-?86

// Walker M. White:  Updated for SDL filesystem

/************************************/
/* Inlcude default library packages */
/************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <cstring>
#include <sys/types.h>
#include <SDL/SDL.h>

using namespace std;

#ifndef _L__SOUND__
#define _L__SOUND__

/****************/
/* Sound Errors */
/****************/
// General error
#define SOUND_ERROR_OK                0
#define SOUND_ERROR_FINISH           -1

// Device error (for player)
#define SOUND_ERROR_DEVOPENFAIL       1
#define SOUND_ERROR_DEVBUSY           2
#define SOUND_ERROR_DEVBADBUFFERSIZE  3
#define SOUND_ERROR_DEVCTRLERROR      4

// Sound file (for reader)
#define SOUND_ERROR_FILEOPENFAIL      5
#define SOUND_ERROR_FILEREADFAIL      6

// Network
#define SOUND_ERROR_UNKNOWNPROXY      7
#define SOUND_ERROR_UNKNOWNHOST       8
#define SOUND_ERROR_SOCKET            9
#define SOUND_ERROR_CONNECT          10
#define SOUND_ERROR_FDOPEN           11
#define SOUND_ERROR_HTTPFAIL         12
#define SOUND_ERROR_HTTPWRITEFAIL    13
#define SOUND_ERROR_TOOMANYRELOC     14

// Miscellneous (for translater)
#define SOUND_ERROR_MEMORYNOTENOUGH  15
#define SOUND_ERROR_EOF              16
#define SOUND_ERROR_BAD              17

#define SOUND_ERROR_THREADFAIL       18

#define SOUND_ERROR_UNKNOWN          19


/**************************/
/* Define values for MPEG */
/**************************/
#define SCALEBLOCK     12
#define CALCBUFFERSIZE 512
#define MAXSUBBAND     32
#define MAXCHANNEL     2
#define MAXTABLE       2
#define SCALE          32768
#define MAXSCALE       (SCALE-1)
#define MINSCALE       (-SCALE)
#define RAWDATASIZE    (2*2*32*SSLIMIT)

#define LS 0
#define RS 1

#define SSLIMIT      18
#define SBLIMIT      32

#define WINDOWSIZE    4096

// Huffmancode
#define HTN 34


/*******************************************/
/* Define values for Microsoft WAVE format */
/*******************************************/
#define RIFF		0x46464952
#define WAVE		0x45564157
#define FMT		0x20746D66
#define DATA		0x61746164
#define PCM_CODE	1
#define WAVE_MONO	1
#define WAVE_STEREO	2

#define MODE_MONO   0
#define MODE_STEREO 1

/********************/
/* Type definitions */
/********************/
typedef float REAL;

typedef struct _waveheader {
  unsigned int	main_chunk;  // 'RIFF'
  unsigned int	length;      // filelen
  unsigned int	chunk_type;  // 'WAVE'

  unsigned int	sub_chunk;   // 'fmt '
  unsigned int	sc_len;      // length of sub_chunk, =16
  unsigned short format;      // should be 1 for PCM-code
  unsigned short modus;       // 1 Mono, 2 Stereo
  unsigned int	sample_fq;   // frequence of sample
  unsigned int	byte_p_sec;
  unsigned short byte_p_spl;  // samplesize; 1 or 2 bytes
  unsigned short bit_p_spl;   // 8, 12 or 16 bit

  unsigned int	data_chunk;  // 'data'
  unsigned int	data_length; // samplecount
}WAVEHEADER;

typedef struct
{
  bool         generalflag;
  unsigned int part2_3_length;
  unsigned int big_values;
  unsigned int global_gain;
  unsigned int scalefac_compress;
  unsigned int window_switching_flag;
  unsigned int block_type;
  unsigned int mixed_block_flag;
  unsigned int table_select[3];
  unsigned int subblock_gain[3];
  unsigned int region0_count;
  unsigned int region1_count;
  unsigned int preflag;
  unsigned int scalefac_scale;
  unsigned int count1table_select;
}layer3grinfo;

typedef struct
{
  unsigned main_data_begin;
  unsigned private_bits;
  struct
  {
    unsigned scfsi[4];
    layer3grinfo gr[2];
  }ch[2];
}layer3sideinfo;

typedef struct
{
  int l[23];            /* [cb] */
  int s[3][13];         /* [window][cb] */
}layer3scalefactor;     /* [ch] */

typedef struct
{
  int tablename;
  unsigned int xlen,ylen;
  unsigned int linbits;
  unsigned int treelen;
  const unsigned int (*val)[2];
}HUFFMANCODETABLE;

/*********************************/
/* Sound input interface classes */
/*********************************/
// Superclass for Inputbitstream // Yet, Temporary
class Soundinputstream
{
public:
    Soundinputstream();
    ~Soundinputstream();

    int geterrorcode(void)  {return __errorcode;};

    bool open(const char *filename);
    int  getbytedirect(void);
    bool _readbuffer(char *buffer,int size);
    bool eof(void);
    int  getblock(char *buffer,int size);

    int  getsize(void);
    int  getposition(void);
    void setposition(int pos);

protected:
    void seterrorcode(int errorcode) {__errorcode=errorcode;};

private:
    int __errorcode;
    SDL_RWops *fp;
    Sint64    size;
};


/*********************************/
/* Data format converter classes */
/*********************************/

// Class for Mpeg layer3
class Mpegbitwindow
{
public:
  Mpegbitwindow(){bitindex=point=0;};

  void initialize(void)  {bitindex=point=wordindex=0;};
  int  gettotalbit(void) const {return bitindex;};
  void putbyte(int c)    {buffer[point&(WINDOWSIZE-1)]=c;point++;};
  void wrap(void);
  void rewind(int bits)  {bitindex-=bits;};
  void forward(int bits) {bitindex+=bits;};
  int getbit(void) {
      // Deprecated in C+11
      // register int r=(buffer[bitindex>>3]>>(7-(bitindex&7)))&1;
      int r=(buffer[bitindex>>3]>>(7-(bitindex&7)))&1;
      bitindex++;
      return r;
  }
  int getbits9(int bits)
  {
      // Deprecated in C+11
      // register unsigned short a;
      unsigned short a;
      { int offset=bitindex>>3;

        a=(((unsigned char)buffer[offset])<<8) | ((unsigned char)buffer[offset+1]);
      }
      a<<=(bitindex&7);
      bitindex+=bits;
      return (int)((unsigned int)(a>>(16-bits)));
  }
  int  getbits(int bits);

private:
  int  point,bitindex;
	int wordindex;
  unsigned char buffer[2*WINDOWSIZE];
};



// Class for converting mpeg format to raw format
class Mpegtoraw
{
  /*****************************/
  /* Constant tables for layer */
  /*****************************/
private:
  static const int bitrate[3][3][15],frequencies[3][3];
  static const REAL scalefactorstable[64];
  static const HUFFMANCODETABLE ht[HTN];
  static const REAL filter[512];
  REAL scaledfilter[512];
  static REAL hcos_64[16],hcos_32[8],hcos_16[4],hcos_8[2],hcos_4;

  /*************************/
  /* MPEG header variables */
  /*************************/
private:
  int layer,protection,bitrateindex,padding,extendedmode;
  enum _mpegversion  {mpeg1,mpeg2,mpeg25}                        version;
  enum _mode    {fullstereo,joint,dual,single}                   mode;
  enum _frequency {frequency44100,frequency48000,frequency32000} frequency;

  /*******************************************/
  /* Functions getting MPEG header variables */
  /*******************************************/
public:
  // General
  int  getversion(void)   const {return version;};
  int  getlayer(void)     const {return layer;};
  bool getcrccheck(void)  const {return (!protection);};
  // Stereo or not
  int  getmode(void)      const {return mode;};
  bool isstereo(void)     const {return (getmode()!=single);};
  // Frequency and bitrate
  int  getfrequency(void) const {return frequencies[version][frequency];};
  int  getbitrate(void)   const {return bitrate[version][layer-1][bitrateindex];};

  /***************************************/
  /* Interface for setting music quality */
  /***************************************/
private:
  bool forcetomonoflag;
  int  downfrequency;

public:
  void setforcetomono(bool flag);
  void setdownfrequency(int value);

  /******************************************/
  /* Functions getting other MPEG variables */
  /******************************************/
public:
  bool getforcetomono(void);
  int  getdownfrequency(void);
  int  getpcmperframe(void);

  /******************************/
  /* Frame management variables */
  /******************************/
private:
  int currentframe,totalframe;
  int decodeframe;
  int *frameoffsets;

  /******************************/
  /* Frame management functions */
  /******************************/
public:
  int  getcurrentframe(void) const {return currentframe;};
  int  gettotalframe(void)   const {return totalframe;};
  void setframe(int framenumber);

  /***************************************/
  /* Variables made by MPEG-Audio header */
  /***************************************/
private:
  int tableindex,channelbitrate;
  int stereobound,subbandnumber,inputstereo,outputstereo;
  REAL scalefactor;
  int framesize;

  /*******************/
  /* Mpegtoraw class */
  /*******************/
public:
  Mpegtoraw(Soundinputstream *loader);
  ~Mpegtoraw();
  void initialize(char *filename);
  long run(short* buffer,int frames);
  int  geterrorcode(void) {return __errorcode;};
  void clearbuffer(void);

private:
  int __errorcode;
  bool seterrorcode(int errorno){__errorcode=errorno;return false;};

  /*****************************/
  /* Loading MPEG-Audio stream */
  /*****************************/
private:
  Soundinputstream *loader;   // Interface
  union
  {
    unsigned char store[4];
    unsigned int  current;
  }u;
  unsigned char buffer[4096];
  int  bitindex;
  bool fillbuffer(int size){bitindex=0;return loader->_readbuffer((char*)buffer,size);};
  void sync(void)  {bitindex=(bitindex+7)&0xFFFFFFF8;};
  bool issync(void){return (bitindex&7);};
  int getbyte(void);
  int getbits(int bits);
  int getbits9(int bits);
  int getbits8(void);
  int getbit(void);

  /********************/
  /* Global variables */
  /********************/
private:
  int lastfrequency,laststereo;

  // for Layer3
  int layer3slots,layer3framestart,layer3part2start;
  REAL prevblck[2][2][SBLIMIT][SSLIMIT];
  int currentprevblock;
  layer3sideinfo sideinfo;
  layer3scalefactor scalefactors[2];

  Mpegbitwindow bitwindow;
  int wgetbit  (void)    {return bitwindow.getbit  ();    }
  int wgetbits9(int bits){return bitwindow.getbits9(bits);}
  int wgetbits (int bits){return bitwindow.getbits (bits);}


  /*************************************/
  /* Decoding functions for each layer */
  /*************************************/
private:
  bool loadheader(int);

  //
  // Subbandsynthesis
  //
  REAL calcbufferL[2][CALCBUFFERSIZE],calcbufferR[2][CALCBUFFERSIZE];
  int  currentcalcbuffer,calcbufferoffset;

  void computebuffer(REAL *fraction,REAL buffer[2][CALCBUFFERSIZE]);
  void generatesingle(void);
  void generate(void);
  void subbandsynthesis(REAL *fractionL,REAL *fractionR);

  void computebuffer_2(REAL *fraction,REAL buffer[2][CALCBUFFERSIZE]);
  void generatesingle_2(void);
  void generate_2(void);
  void subbandsynthesis_2(REAL *fractionL,REAL *fractionR);

  // Extarctor
  void extractlayer1(void);    // MPEG-1
  void extractlayer2(void);
  void extractlayer3(void);
  void extractlayer3_2(void);  // MPEG-2


  // Functions for layer 3
  void layer3initialize(void);
  bool layer3getsideinfo(void);
  bool layer3getsideinfo_2(void);
  void layer3getscalefactors(int ch,int gr);
  void layer3getscalefactors_2(int ch);
  void layer3huffmandecode(int ch,int gr,int out[SBLIMIT][SSLIMIT]);
  REAL layer3twopow2(int scale,int preflag,int pretab_offset,int l);
  REAL layer3twopow2_1(int a,int b,int c);
  void layer3dequantizesample(int ch,int gr,int   in[SBLIMIT][SSLIMIT],
			                    REAL out[SBLIMIT][SSLIMIT]);
  void layer3fixtostereo(int gr,REAL  in[2][SBLIMIT][SSLIMIT]);
  void layer3reorderandantialias(int ch,int gr,REAL  in[SBLIMIT][SSLIMIT],
				               REAL out[SBLIMIT][SSLIMIT]);

  void layer3hybrid(int ch,int gr,REAL in[SBLIMIT][SSLIMIT],
		                  REAL out[SSLIMIT][SBLIMIT]);
  
  void huffmandecoder_1(const HUFFMANCODETABLE *h,int *x,int *y);
  void huffmandecoder_2(const HUFFMANCODETABLE *h,int *x,int *y,int *v,int *w);

  /********************/
  /* Playing raw data */
  /********************/
private:
  int       rawdataoffset;
  short int *rawdata;

  void clearrawdata(void)    {rawdataoffset=0;};
  void putraw(short int pcm) {rawdata[rawdataoffset++]=pcm;};
  void flushrawdata(void);

};



#endif
