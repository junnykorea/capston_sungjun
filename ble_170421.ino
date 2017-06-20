#include <stdio.h>
#include <stdlib.h>

#define HM10 Serial3


#if 1
#define _LEA128_
#else
#if 1
#define _LEA192_
#else
#define _LEA256_
#endif
#endif


typedef unsigned char u8;
typedef unsigned char uint8;
typedef uint32_t u32;
typedef uint32_t uint32;
unsigned char send_data[20];
typedef struct sha256_context
{
  uint32 total[2];
  uint32 state[8];
  uint8 buffer[64];
} sha256_context;


typedef enum macAlgorithmEnum
{
  MAC_NONE,
  MAC_SHA256
} MAC_ALGORITHM;

typedef enum cryptoAlgorithmEnum
{
  CRYPTO_NONE,
  CRYPTO_PRESENT_ECB,
  CRYPTO_LEA128_ECB
  
} CRYPTO_ALGORITHM;



typedef struct dg_sessionKey
{
  CRYPTO_ALGORITHM encAgtm;
  MAC_ALGORITHM hasAgtm;  
  unsigned char sessionKey[16];
  unsigned char sessionKeyLen;
  
}DG_SESSIONKEY;


DG_SESSIONKEY DG_SK = {
                        CRYPTO_NONE,
                        MAC_NONE,
                        { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f },
                        0
  };




#define CRYPTO_LEA_BLOCKSIZE 16

#define data_ble 0x01
#define uidrand 0x02
#define sres 0x03
#define situp 0x04
#define HMAC_BLOCKSIZE 64
#define IPAD 0x36 
#define OPAD 0x5C 

#define GET_UINT32(n,b,i)                       \
{                                               \
    (n) = ( (uint32) (b)[(i)    ] << 24 )       \
        | ( (uint32) (b)[(i) + 1] << 16 )       \
        | ( (uint32) (b)[(i) + 2] <<  8 )       \
        | ( (uint32) (b)[(i) + 3]       );      \
}

#define PUT_UINT32(n,b,i)                       \
{                                               \
    (b)[(i)    ] = (uint8) ( (n) >> 24 );       \
    (b)[(i) + 1] = (uint8) ( (n) >> 16 );       \
    (b)[(i) + 2] = (uint8) ( (n) >>  8 );       \
    (b)[(i) + 3] = (uint8) ( (n)       );       \
}





#if defined(_LEA128_)
#define LEA_NUM_RNDS    24
#define LEA_KEY_BYTE_LEN  16
#endif

#if defined(_LEA192_)
#define LEA_NUM_RNDS    28
#define LEA_KEY_BYTE_LEN  24
#endif

#if defined(_LEA256_)
#define LEA_NUM_RNDS    32
#define LEA_KEY_BYTE_LEN  32
#endif

#define LEA_BLK_BYTE_LEN  16
#define LEA_RNDKEY_WORD_LEN 6
#define AUTH_USER_ID_LENGTH          10
#if defined _MSC_VER
#define ROR(W,i) _lrotr(W, i)
#define ROL(W,i) _lrotl(W, i)
#else
#define ROR(W,i) (((W)>>(i)) | ((W)<<(32-(i))))
#define ROL(W,i) (((W)<<(i)) | ((W)>>(32-(i))))
#endif

#define u32_in(x)            (*(u32*)(x))
#define u32_out(x, v)        {*((u32*)(x)) = (v);}
#define AUTH_RAND_LENGTH            4
#define AUTH_MSG_LENGTH             (AUTH_USER_ID_LENGTH + AUTH_RAND_LENGTH)



void Show_jun(u32 *src, int word_len);

void Show_junRndKeys(u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN]);
void LEA_Test();
void LEA_Keyschedule(u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN], const u8 pbKey[LEA_KEY_BYTE_LEN]);
void LEA_EncryptBlk(u8 pbDst[LEA_BLK_BYTE_LEN], const u8 pbSrc[LEA_BLK_BYTE_LEN], const u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN]);
void LEA_DecryptBlk(u8 pbDst[LEA_BLK_BYTE_LEN], const u8 pbSrc[LEA_BLK_BYTE_LEN], const u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN]);
void lea_enc(unsigned char *key, unsigned char keyLen, unsigned char *plain, unsigned char plainLen, unsigned char *cipher, unsigned char *cipherLen);
void hmac(unsigned char macAlgorithm, unsigned char *key, unsigned char keyLen, unsigned char *msg, unsigned char msgLen, unsigned char *result, unsigned char  *resultLen);
void mac(int macAlgorithm, unsigned char *msg, int msgLeng, unsigned char *result);
void sha256_process(sha256_context *ctx, u8 data[64]);
void sha256_init(sha256_context *ctx);
void sha256_update(sha256_context *ctx, uint8 *input, uint32 length);
void send_ble(unsigned char *data, unsigned char function_code, int size);
void sha256_finish(sha256_context *ctx, uint8 digest[32]);


//라운드키
u32 rndkeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN] = { 0x0, };
u8 src1[LEA_BLK_BYTE_LEN] = { 0x0, };
u8 src2[LEA_BLK_BYTE_LEN] = { 0x0, };
u8 src3[LEA_BLK_BYTE_LEN] = { 0x0, };

u8 key[32] = {
  0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0,
  0xf0, 0xe1, 0xd2, 0xc3, 0xb4, 0xa5, 0x96, 0x87, 0x78, 0x69, 0x5a, 0x4b, 0x3c, 0x2d, 0x1e, 0x0f
};

typedef struct dg_updateKeyList
{
  unsigned char userId[AUTH_USER_ID_LENGTH];//40
  unsigned char userIdLen;
  
  unsigned char encAgtm;
  unsigned char hasAgtm;  
  unsigned char updateKey[16];
  unsigned char updateKeyLen;
  
}DG_UPDATEKEY_LIST;


DG_UPDATEKEY_LIST DG_UKList[1] = { 
  {
    {
      'd', 'g', 't', 'e', 's', 't', 0x00, 0x00, 0x00, 0x00
    }, 
    6,
    0x02, 
    0x01, 
    {
      'u', 'p', 'd', 'a', 't', 'e', 'k', 'e', 'y', 'l', 
      'e', 'a', 0x00, 0x00, 0x00, 0x00
    },
    16
  } 
};


static uint8 sha256_padding[64] =
{
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

int count=0;

int now_mode = 0; //
unsigned char hamc_result[32] = {0, };
int decipherLen = 0;
unsigned char DG_usingUKIdx    = 0;
  unsigned char hmacSum[32] = {0,};
    unsigned char hmacSumSize = 0;


void setup()
{
  // delay(1000);
  Serial.begin(38400);
  HM10.begin(9600);
  

  // Serial.write("\nTest Start\n");
}

void loop()
{

  srand(analogRead(0)); 



  long start;

while(1){
 
    
  



  u8 real_data[40];//function코드 포함
  u8 receive_data[40];




  if (HM10.available())
  {
    unsigned char temp[2];
    delay(22);
    temp[0] = (unsigned char)HM10.read();

    temp[1] = (unsigned char)HM10.read();

    Serial.print("syn data:");

    Serial.print(temp[0], HEX);
    Serial.print(temp[1], HEX);
    Serial.println();

Serial.print("count:");
Serial.print(count,DEC);
Serial.println();
    if (temp[0] == 0xf3 && temp[1] == 0x12)
    {

        
      
count++;
      if(count==1)
      {
start=millis();
}
if(count==100)
{
long endtime=millis();
Serial.print("-------------endtime-------:");
Serial.print((endtime-start)/100,DEC);
Serial.print("(단위:100MS)");

Serial.println();
  
}
      char receive_size = 0;
      char received_size = 0;
      unsigned char function_code;
          unsigned char au_msg[AUTH_MSG_LENGTH] = {0, };
    unsigned char au_msgLen = 0;
  
      receive_size = (unsigned char)HM10.read();
      Serial.print("receive_size:");
      Serial.println(receive_size, DEC);

   
      
   


      while (received_size < receive_size + 1)
      {

        if (HM10.available()) {
          unsigned char sendunchar;
          sendunchar = (unsigned char)HM10.read();
         
          receive_data[received_size] = sendunchar;

          received_size++;
        }
      }
      function_code=receive_data[0];
      memcpy(real_data,receive_data+1,receive_size);
      Serial.println();
      Serial.print("function code:");
      Serial.println(function_code, DEC);
  
   Serial.print("real_receive data:");
    for(int k=0;k<receive_size;k++)
    {
Serial.print(real_data[k],HEX);
Serial.print("  ");      
    }
    Serial.println();

    
      switch (function_code)
      {


        
      case 0x01:
      {
        
  u8 decry_data[50]={0x00,};
      Serial.println();
      Serial.print("received:");
      for(int k=0;k<receive_size;k++)
      Serial.print(real_data[k],HEX);

      //받은값 확인

      //시
      decipherLen=0;
      for (unsigned char i = 0; i <receive_size; i += CRYPTO_LEA_BLOCKSIZE)
      {
      unsigned char blockCipher[CRYPTO_LEA_BLOCKSIZE] = {0,};
      unsigned char blockDecipher[CRYPTO_LEA_BLOCKSIZE] = {0,};

      memcpy(blockCipher, real_data + i, CRYPTO_LEA_BLOCKSIZE);


      LEA_DecryptBlk(blockDecipher, blockCipher,rndkeys); //blockDecipher 는 블록 복호화
      memcpy(decry_data + i, blockDecipher, CRYPTO_LEA_BLOCKSIZE);

      decipherLen += CRYPTO_LEA_BLOCKSIZE;//decipherLen은 전체 블록
      }

      decipherLen=unpad(decry_data, decipherLen, CRYPTO_LEA_BLOCKSIZE);
      //끝
      Serial.println();
      Serial.println("decipherLen:");
      Serial.println(decipherLen,DEC);

      Serial.println("decry:");
      for(int k=0;k<decipherLen;k++){
      Serial.print(decry_data[k],HEX);
      Serial.print("  ");
      }
      //복호화
      Serial.println();




      }

    {
      //암호화 시작
      unsigned char plain[18] = { 0xA0,0xc1,0x04,0x03,0x04,0x15,0x56,0x07,0x08,0x39,0x0a,0x0b,0x0c,0x0d,0x0e,0x0F,0x0c };//,0x00,0x00,0x00,0x00,;
      unsigned char cipher[50];
      char cipherLen;
        unsigned char resultLen = 0;
      unsigned char send_data[72];
      char blockPlain[16];
      char blockCipher[16];
      cipherLen = pad(plain, 15, CRYPTO_LEA_BLOCKSIZE, cipher);
      //캡디 plainlen 2번쨰 변수 수정  
      for (unsigned char i = 0; i < cipherLen; i += CRYPTO_LEA_BLOCKSIZE)
      {
        unsigned char blockPlain[CRYPTO_LEA_BLOCKSIZE] = { 0, };
        unsigned char blockCipher[CRYPTO_LEA_BLOCKSIZE] = { 0, };

        memcpy(blockPlain, cipher + i, CRYPTO_LEA_BLOCKSIZE);


        LEA_EncryptBlk(blockCipher, blockPlain, rndkeys);


        memcpy(cipher + i, blockCipher, CRYPTO_LEA_BLOCKSIZE);
      }
send_ble(cipher,0x01,CRYPTO_LEA_BLOCKSIZE);
   
        //암호화 끝
    

    }
        break;
      case 0x02:
        
   unsigned char rand_num[4];

      
        for (int i = 0; i < 4; i++)
        {
          rand_num[i] = rand() & 0xff;

        }

    memcpy(au_msg, real_data , receive_size ); //copy userId
    au_msgLen =receive_size;
        
    memcpy(au_msg+receive_size, rand_num, AUTH_RAND_LENGTH); //copy rand number
    au_msgLen += AUTH_RAND_LENGTH;


hmac(DG_UKList[DG_usingUKIdx].hasAgtm, DG_UKList[DG_usingUKIdx].updateKey, DG_UKList[DG_usingUKIdx].updateKeyLen, au_msg, au_msgLen, hmacSum, &hmacSumSize);

    Serial.print("hmac: "); 
for(int k=0;k<hmacSumSize;k++)
{

Serial.print(hmacSum[k],HEX);
Serial.print("  ");  
}
memcpy(key,hmacSum+4,16);
LEA_Keyschedule(rndkeys, key);
Serial.println();
        
        send_ble(rand_num, 02, 4);
      


        break;
      case 0x03:
Serial.print("SRES:");

for(int kk=0;kk<receive_size;kk++)
{
Serial.print(real_data[kk],HEX);
Serial.print("  ");
}
Serial.println();
if(memcmp(hmacSum,real_data,4)==0)
{
  unsigned char hexdata[2]={0x01};
send_ble(hexdata,0x03,1);
  
}
Serial.println();
Serial.println();

Serial.print("key:");
for(int k=0;k<16;k++)
{
  Serial.print(key[k],HEX);
  Serial.print(" ");
  
}

Serial.println();
        break;
      case 0x04:

        break;
        
      case 0x05:

        break;



      }


      /*
      u8 plain[50]={0x00,};
      Serial.println();
      Serial.print("received:");
      for(int k=0;k<receive_size;k++)
      Serial.print(real_data[k],HEX);

      //받은값 확인

      //시
      decipherLen=0;
      for (unsigned char i = 0; i <receive_size; i += CRYPTO_LEA_BLOCKSIZE)
      {
      unsigned char blockCipher[CRYPTO_LEA_BLOCKSIZE] = {0,};
      unsigned char blockDecipher[CRYPTO_LEA_BLOCKSIZE] = {0,};

      memcpy(blockCipher, real_data + i, CRYPTO_LEA_BLOCKSIZE);


      LEA_DecryptBlk(blockDecipher, blockCipher,rndkeys); //blockDecipher 는 블록 복호화
      memcpy(real_data + i, blockDecipher, CRYPTO_LEA_BLOCKSIZE);

      decipherLen += CRYPTO_LEA_BLOCKSIZE;//decipherLen은 전체 블록
      }

      decipherLen=unpad(real_data, decipherLen, CRYPTO_LEA_BLOCKSIZE);
      //끝
      Serial.println();
      Serial.println("decipherLen:");
      Serial.println(decipherLen,DEC);

      Serial.println("decry:");
      for(int k=0;k<decipherLen;k++)
      Serial.print(real_data[k],HEX);
      //복호화
      Serial.println();
      */


    }


  }
}
}

void LEA_Keyschedule(u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN], const u8 pbKey[LEA_KEY_BYTE_LEN])
{

#if defined(_LEA128_)

  u32 delta[4] = { 0xc3efe9db, 0x44626b02, 0x79e27c8a, 0x78df30ec };
  u32 T[4] = { 0x0, };

  T[0] = u32_in(pbKey);
  T[1] = u32_in(pbKey + 4);
  T[2] = u32_in(pbKey + 8);
  T[3] = u32_in(pbKey + 12);

  for (int i = 0; i < LEA_NUM_RNDS; i++)
  {
    T[0] = ROL(T[0] + ROL(delta[i & 3], i), 1);
    T[1] = ROL(T[1] + ROL(delta[i & 3], i + 1), 3);
    T[2] = ROL(T[2] + ROL(delta[i & 3], i + 2), 6);
    T[3] = ROL(T[3] + ROL(delta[i & 3], i + 3), 11);

    pdRndKeys[i][0] = T[0];
    pdRndKeys[i][1] = T[1];
    pdRndKeys[i][2] = T[2];
    pdRndKeys[i][3] = T[1];
    pdRndKeys[i][4] = T[3];
    pdRndKeys[i][5] = T[1];
  }
#endif

#if defined(_LEA192_)

  u32 delta[6] = { 0xc3efe9db, 0x44626b02, 0x79e27c8a, 0x78df30ec, 0x715ea49e, 0xc785da0a };
  u32 T[6] = { 0x0, };

  T[0] = u32_in(pbKey);
  T[1] = u32_in(pbKey + 4);
  T[2] = u32_in(pbKey + 8);
  T[3] = u32_in(pbKey + 12);
  T[4] = u32_in(pbKey + 16);
  T[5] = u32_in(pbKey + 20);

  for (int i = 0; i < LEA_NUM_RNDS; i++)
  {
    T[0] = ROL(T[0] + ROL(delta[i % 6], i & 0x1f), 1);
    T[1] = ROL(T[1] + ROL(delta[i % 6], (i + 1) & 0x1f), 3);
    T[2] = ROL(T[2] + ROL(delta[i % 6], (i + 2) & 0x1f), 6);
    T[3] = ROL(T[3] + ROL(delta[i % 6], (i + 3) & 0x1f), 11);
    T[4] = ROL(T[4] + ROL(delta[i % 6], (i + 4) & 0x1f), 13);
    T[5] = ROL(T[5] + ROL(delta[i % 6], (i + 5) & 0x1f), 17);

    pdRndKeys[i][0] = T[0];
    pdRndKeys[i][1] = T[1];
    pdRndKeys[i][2] = T[2];
    pdRndKeys[i][3] = T[3];
    pdRndKeys[i][4] = T[4];
    pdRndKeys[i][5] = T[5];
  }

#endif

#if defined(_LEA256_)

  u32 delta[8] = { 0xc3efe9db, 0x44626b02, 0x79e27c8a, 0x78df30ec, 0x715ea49e, 0xc785da0a, 0xe04ef22a, 0xe5c40957 };
  u32 T[8] = { 0x0, };

  T[0] = u32_in(pbKey);
  T[1] = u32_in(pbKey + 4);
  T[2] = u32_in(pbKey + 8);
  T[3] = u32_in(pbKey + 12);
  T[4] = u32_in(pbKey + 16);
  T[5] = u32_in(pbKey + 20);
  T[6] = u32_in(pbKey + 24);
  T[7] = u32_in(pbKey + 28);

  for (int i = 0; i < LEA_NUM_RNDS; i++)
  {
    T[(6 * i) & 7] = ROL(T[(6 * i) & 7] + ROL(delta[i & 7], i & 0x1f), 1);
    T[(6 * i + 1) & 7] = ROL(T[(6 * i + 1) & 7] + ROL(delta[i & 7], (i + 1) & 0x1f), 3);
    T[(6 * i + 2) & 7] = ROL(T[(6 * i + 2) & 7] + ROL(delta[i & 7], (i + 2) & 0x1f), 6);
    T[(6 * i + 3) & 7] = ROL(T[(6 * i + 3) & 7] + ROL(delta[i & 7], (i + 3) & 0x1f), 11);
    T[(6 * i + 4) & 7] = ROL(T[(6 * i + 4) & 7] + ROL(delta[i & 7], (i + 4) & 0x1f), 13);
    T[(6 * i + 5) & 7] = ROL(T[(6 * i + 5) & 7] + ROL(delta[i & 7], (i + 5) & 0x1f), 17);

    pdRndKeys[i][0] = T[(6 * i) & 7];
    pdRndKeys[i][1] = T[(6 * i + 1) & 7];
    pdRndKeys[i][2] = T[(6 * i + 2) & 7];
    pdRndKeys[i][3] = T[(6 * i + 3) & 7];
    pdRndKeys[i][4] = T[(6 * i + 4) & 7];
    pdRndKeys[i][5] = T[(6 * i + 5) & 7];
  }
#endif

}


void LEA_EncryptBlk(u8 pbDst[LEA_BLK_BYTE_LEN], const u8 pbSrc[LEA_BLK_BYTE_LEN], const u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN])
{
  u32 X0, X1, X2, X3;
  u32 temp;

  X0 = u32_in(pbSrc);
  X1 = u32_in(pbSrc + 4);
  X2 = u32_in(pbSrc + 8);
  X3 = u32_in(pbSrc + 12);

  for (int i = 0; i < LEA_NUM_RNDS; i++)
  {
    X3 = ROR((X2 ^ pdRndKeys[i][4]) + (X3 ^ pdRndKeys[i][5]), 3);
    X2 = ROR((X1 ^ pdRndKeys[i][2]) + (X2 ^ pdRndKeys[i][3]), 5);
    X1 = ROL((X0 ^ pdRndKeys[i][0]) + (X1 ^ pdRndKeys[i][1]), 9);
    temp = X0;
    X0 = X1; X1 = X2; X2 = X3; X3 = temp;
  }

  u32_out(pbDst, X0);
  u32_out(pbDst + 4, X1);
  u32_out(pbDst + 8, X2);
  u32_out(pbDst + 12, X3);
}
void LEA_DecryptBlk(u8 pbDst[LEA_BLK_BYTE_LEN], const u8 pbSrc[LEA_BLK_BYTE_LEN], const u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN])
{
  u32 X0, X1, X2, X3;
  u32 temp;

  X0 = u32_in(pbSrc);
  X1 = u32_in(pbSrc + 4);
  X2 = u32_in(pbSrc + 8);
  X3 = u32_in(pbSrc + 12);


  for (int i = 0; i < LEA_NUM_RNDS; i++)
  {
    temp = X3;
    X3 = X2;
    X2 = X1;
    X1 = X0;
    X0 = temp;

    X1 = (ROR(X1, 9) - (X0 ^ pdRndKeys[LEA_NUM_RNDS - 1 - i][0])) ^ pdRndKeys[LEA_NUM_RNDS - 1 - i][1];
    X2 = (ROL(X2, 5) - (X1 ^ pdRndKeys[LEA_NUM_RNDS - 1 - i][2])) ^ pdRndKeys[LEA_NUM_RNDS - 1 - i][3];
    X3 = (ROL(X3, 3) - (X2 ^ pdRndKeys[LEA_NUM_RNDS - 1 - i][4])) ^ pdRndKeys[LEA_NUM_RNDS - 1 - i][5];

  }

  u32_out(pbDst, X0);
  u32_out(pbDst + 4, X1);
  u32_out(pbDst + 8, X2);
  u32_out(pbDst + 12, X3);
}


unsigned char pad(unsigned char *source, unsigned char source_len, unsigned char block_size, unsigned char *dest)
{
  unsigned char dest_len = ((source_len / block_size) + 1) * block_size;
  unsigned char pad = 0x01;

  for (unsigned char i = 1; i < (dest_len - source_len); i++)
    pad += 0x01;

  if (source_len % block_size == 0)
  {
    //dest_len += block_size;
    pad = 0x00;
  }


  memset(dest, pad, dest_len);
  memcpy(dest, source, source_len);

  return dest_len;
}


int unpad(unsigned char *source, int source_len, int block_size)
{
  u8 dest_len = source_len - source[source_len - 1];

  if (source[source_len - 1] == 0) //원래 블록에 패딩된 경우
  {
    dest_len -= block_size; //실제 소스길이   
  }

  //memcpy(dest, source, dest_len);

  return  dest_len;
}

void send_ble(unsigned char *data, unsigned char function_code, int size)
{
  unsigned char real_send[40];
  real_send[0] = 0xf3;
  real_send[1] = 0x12;
  real_send[2] = size;
  real_send[3] = function_code;
  memcpy(real_send + 4, data, size);
  HM10.write(real_send, size + 4);

  Serial.print("send data:");
  for (int i = 0; i < size; i++) {
    Serial.print(data[i], HEX);
    Serial.print("  ");
  }
  Serial.println();
}

void hmac(unsigned char macAlgorithm, unsigned char *key, unsigned char keyLen, unsigned char *msg, unsigned char msgLen, unsigned char *result, unsigned char  *resultLen)
{  
  int i;

  int hmacSumSize = 32; // sha256
  unsigned char hmacSum[32];
  unsigned char buffer[96];
  int bufferSize = (msgLen > hmacSumSize) ? msgLen : hmacSumSize; 
  
//  my_printf(0,"key[%d] : ", keyLen);
//  for (i = 0; i < keyLen; i++)
//  {
//    my_printf(0,"%02x ", key[i]);
//  }
//  my_printf(0,"\n");

//  my_printf(0,"msg[%d] : ", msgLen);
//  for (i = 0; i < msgLen; i++)
//  {
//    my_printf(0,"%02x ", msg[i]);
//  }
//  my_printf(0,"\n");

  
  
  memset(buffer, 0, HMAC_BLOCKSIZE); //zero padding

  if (keyLen > HMAC_BLOCKSIZE)
  {
    mac(macAlgorithm, msg, msgLen, buffer);
  }
  else
  {
    memcpy(buffer, key, keyLen);
  }
  
  for (i = 0; i < HMAC_BLOCKSIZE; i++)
  {
    buffer[i] ^= IPAD;
  }

  
  memcpy(buffer + HMAC_BLOCKSIZE, msg, msgLen);
  
  
  mac(macAlgorithm, buffer, HMAC_BLOCKSIZE + msgLen, hmacSum);
  
  for (i = 0; i < HMAC_BLOCKSIZE; i++)
  {
    buffer[i] ^= IPAD ^ OPAD;
  }

  
  memcpy(buffer + HMAC_BLOCKSIZE, hmacSum, hmacSumSize);
    
  mac(macAlgorithm, buffer, HMAC_BLOCKSIZE + hmacSumSize, hmacSum);
  
  *resultLen = hmacSumSize;
  memcpy(result, hmacSum, *resultLen);

//  my_printf(0,"hmac : ");
//  for (i = 0; i < hmacSumSize; i++)
//  {
//    my_printf(0,"%02x ", hmacSum[i]);
//  }
//  my_printf(0,"\n");

}

void mac(int macAlgorithm, unsigned char *msg, int msgLeng, unsigned char *result)
{
  switch (macAlgorithm)
  {
    case MAC_SHA256 :
    {
      sha256_context ctx;

      sha256_init(&ctx);
      sha256_update(&ctx, (u8 *)msg, msgLeng);
      sha256_finish(&ctx, result);

      break;
    }
  }
}


void sha256_init(sha256_context *ctx)
{
  ctx->total[0] = 0;
  ctx->total[1] = 0;

  ctx->state[0] = 0x6A09E667;
  ctx->state[1] = 0xBB67AE85;
  ctx->state[2] = 0x3C6EF372;
  ctx->state[3] = 0xA54FF53A;
  ctx->state[4] = 0x510E527F;
  ctx->state[5] = 0x9B05688C;
  ctx->state[6] = 0x1F83D9AB;
  ctx->state[7] = 0x5BE0CD19;
}

void sha256_process(sha256_context *ctx, u8 data[64])
{
  uint32 temp1, temp2, W[64];
  uint32 A, B, C, D, E, F, G, H;

  GET_UINT32(W[0], data, 0);
  GET_UINT32(W[1], data, 4);
  GET_UINT32(W[2], data, 8);
  GET_UINT32(W[3], data, 12);
  GET_UINT32(W[4], data, 16);
  GET_UINT32(W[5], data, 20);
  GET_UINT32(W[6], data, 24);
  GET_UINT32(W[7], data, 28);
  GET_UINT32(W[8], data, 32);
  GET_UINT32(W[9], data, 36);
  GET_UINT32(W[10], data, 40);
  GET_UINT32(W[11], data, 44);
  GET_UINT32(W[12], data, 48);
  GET_UINT32(W[13], data, 52);
  GET_UINT32(W[14], data, 56);
  GET_UINT32(W[15], data, 60);

#define  SHR(x,n) ((x & 0xFFFFFFFF) >> n)
#define ROTR(x,n) (SHR(x,n) | (x << (32 - n)))

#define S0(x) (ROTR(x, 7) ^ ROTR(x,18) ^  SHR(x, 3))
#define S1(x) (ROTR(x,17) ^ ROTR(x,19) ^  SHR(x,10))

#define S2(x) (ROTR(x, 2) ^ ROTR(x,13) ^ ROTR(x,22))
#define S3(x) (ROTR(x, 6) ^ ROTR(x,11) ^ ROTR(x,25))

#define F0(x,y,z) ((x & y) | (z & (x | y)))
#define F1(x,y,z) (z ^ (x & (y ^ z)))

#define R(t)                                    \
(                                               \
    W[t] = S1(W[t -  2]) + W[t -  7] +          \
           S0(W[t - 15]) + W[t - 16]            \
)

#define P(a,b,c,d,e,f,g,h,x,K)                  \
{                                               \
    temp1 = h + S3(e) + F1(e,f,g) + K + x;      \
    temp2 = S2(a) + F0(a,b,c);                  \
    d += temp1; h = temp1 + temp2;              \
}

  A = ctx->state[0];
  B = ctx->state[1];
  C = ctx->state[2];
  D = ctx->state[3];
  E = ctx->state[4];
  F = ctx->state[5];
  G = ctx->state[6];
  H = ctx->state[7];

  P(A, B, C, D, E, F, G, H, W[0], 0x428A2F98);
  P(H, A, B, C, D, E, F, G, W[1], 0x71374491);
  P(G, H, A, B, C, D, E, F, W[2], 0xB5C0FBCF);
  P(F, G, H, A, B, C, D, E, W[3], 0xE9B5DBA5);
  P(E, F, G, H, A, B, C, D, W[4], 0x3956C25B);
  P(D, E, F, G, H, A, B, C, W[5], 0x59F111F1);
  P(C, D, E, F, G, H, A, B, W[6], 0x923F82A4);
  P(B, C, D, E, F, G, H, A, W[7], 0xAB1C5ED5);
  P(A, B, C, D, E, F, G, H, W[8], 0xD807AA98);
  P(H, A, B, C, D, E, F, G, W[9], 0x12835B01);
  P(G, H, A, B, C, D, E, F, W[10], 0x243185BE);
  P(F, G, H, A, B, C, D, E, W[11], 0x550C7DC3);
  P(E, F, G, H, A, B, C, D, W[12], 0x72BE5D74);
  P(D, E, F, G, H, A, B, C, W[13], 0x80DEB1FE);
  P(C, D, E, F, G, H, A, B, W[14], 0x9BDC06A7);
  P(B, C, D, E, F, G, H, A, W[15], 0xC19BF174);
  P(A, B, C, D, E, F, G, H, R(16), 0xE49B69C1);
  P(H, A, B, C, D, E, F, G, R(17), 0xEFBE4786);
  P(G, H, A, B, C, D, E, F, R(18), 0x0FC19DC6);
  P(F, G, H, A, B, C, D, E, R(19), 0x240CA1CC);
  P(E, F, G, H, A, B, C, D, R(20), 0x2DE92C6F);
  P(D, E, F, G, H, A, B, C, R(21), 0x4A7484AA);
  P(C, D, E, F, G, H, A, B, R(22), 0x5CB0A9DC);
  P(B, C, D, E, F, G, H, A, R(23), 0x76F988DA);
  P(A, B, C, D, E, F, G, H, R(24), 0x983E5152);
  P(H, A, B, C, D, E, F, G, R(25), 0xA831C66D);
  P(G, H, A, B, C, D, E, F, R(26), 0xB00327C8);
  P(F, G, H, A, B, C, D, E, R(27), 0xBF597FC7);
  P(E, F, G, H, A, B, C, D, R(28), 0xC6E00BF3);
  P(D, E, F, G, H, A, B, C, R(29), 0xD5A79147);
  P(C, D, E, F, G, H, A, B, R(30), 0x06CA6351);
  P(B, C, D, E, F, G, H, A, R(31), 0x14292967);
  P(A, B, C, D, E, F, G, H, R(32), 0x27B70A85);
  P(H, A, B, C, D, E, F, G, R(33), 0x2E1B2138);
  P(G, H, A, B, C, D, E, F, R(34), 0x4D2C6DFC);
  P(F, G, H, A, B, C, D, E, R(35), 0x53380D13);
  P(E, F, G, H, A, B, C, D, R(36), 0x650A7354);
  P(D, E, F, G, H, A, B, C, R(37), 0x766A0ABB);
  P(C, D, E, F, G, H, A, B, R(38), 0x81C2C92E);
  P(B, C, D, E, F, G, H, A, R(39), 0x92722C85);
  P(A, B, C, D, E, F, G, H, R(40), 0xA2BFE8A1);
  P(H, A, B, C, D, E, F, G, R(41), 0xA81A664B);
  P(G, H, A, B, C, D, E, F, R(42), 0xC24B8B70);
  P(F, G, H, A, B, C, D, E, R(43), 0xC76C51A3);
  P(E, F, G, H, A, B, C, D, R(44), 0xD192E819);
  P(D, E, F, G, H, A, B, C, R(45), 0xD6990624);
  P(C, D, E, F, G, H, A, B, R(46), 0xF40E3585);
  P(B, C, D, E, F, G, H, A, R(47), 0x106AA070);
  P(A, B, C, D, E, F, G, H, R(48), 0x19A4C116);
  P(H, A, B, C, D, E, F, G, R(49), 0x1E376C08);
  P(G, H, A, B, C, D, E, F, R(50), 0x2748774C);
  P(F, G, H, A, B, C, D, E, R(51), 0x34B0BCB5);
  P(E, F, G, H, A, B, C, D, R(52), 0x391C0CB3);
  P(D, E, F, G, H, A, B, C, R(53), 0x4ED8AA4A);
  P(C, D, E, F, G, H, A, B, R(54), 0x5B9CCA4F);
  P(B, C, D, E, F, G, H, A, R(55), 0x682E6FF3);
  P(A, B, C, D, E, F, G, H, R(56), 0x748F82EE);
  P(H, A, B, C, D, E, F, G, R(57), 0x78A5636F);
  P(G, H, A, B, C, D, E, F, R(58), 0x84C87814);
  P(F, G, H, A, B, C, D, E, R(59), 0x8CC70208);
  P(E, F, G, H, A, B, C, D, R(60), 0x90BEFFFA);
  P(D, E, F, G, H, A, B, C, R(61), 0xA4506CEB);
  P(C, D, E, F, G, H, A, B, R(62), 0xBEF9A3F7);
  P(B, C, D, E, F, G, H, A, R(63), 0xC67178F2);

  ctx->state[0] += A;
  ctx->state[1] += B;
  ctx->state[2] += C;
  ctx->state[3] += D;
  ctx->state[4] += E;
  ctx->state[5] += F;
  ctx->state[6] += G;
  ctx->state[7] += H;
}

void sha256_update(sha256_context *ctx, uint8 *input, uint32 length)
{
  uint32 left, fill;

  if (!length) return;

  left = ctx->total[0] & 0x3F;
  fill = 64 - left;

  ctx->total[0] += length;
  ctx->total[0] &= 0xFFFFFFFF;

  if (ctx->total[0] < length)
    ctx->total[1]++;

  if (left && length >= fill)
  {
    memcpy((void *)(ctx->buffer + left),
      (void *)input, fill);
    sha256_process(ctx, ctx->buffer);
    length -= fill;
    input += fill;
    left = 0;
  }

  while (length >= 64)
  {
    sha256_process(ctx, input);
    length -= 64;
    input += 64;
  }

  if (length)
  {
    memcpy((void *)(ctx->buffer + left),
      (void *)input, length);
  }
}
void sha256_finish(sha256_context *ctx, uint8 digest[32])
{
 uint32 last, padn;
  uint32 high, low;
  uint8 msglen[8];

  high = (ctx->total[0] >> 29)
    | (ctx->total[1] << 3);
  low = (ctx->total[0] << 3);

  PUT_UINT32(high, msglen, 0);
  PUT_UINT32(low, msglen, 4);

  last = ctx->total[0] & 0x3F;
  padn = (last < 56) ? (56 - last) : (120 - last);

  sha256_update(ctx, sha256_padding, padn);
  sha256_update(ctx, msglen, 8);

  PUT_UINT32(ctx->state[0], digest, 0);
  PUT_UINT32(ctx->state[1], digest, 4);
  PUT_UINT32(ctx->state[2], digest, 8);
  PUT_UINT32(ctx->state[3], digest, 12);
  PUT_UINT32(ctx->state[4], digest, 16);
  PUT_UINT32(ctx->state[5], digest, 20);
  PUT_UINT32(ctx->state[6], digest, 24);
  PUT_UINT32(ctx->state[7], digest, 28);
}


