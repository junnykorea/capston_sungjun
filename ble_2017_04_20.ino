#include <stdio.h>

#define hm10 Serial3



#if 1
#define _LEA128_
#else
#if 1
#define _LEA192_
#else
#define _LEA256_
#endif
#endif


#define CRYPTO_LEA_BLOCKSIZE 16


typedef uint8_t u8;

typedef uint32_t u32;
unsigned char send_data[20];



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

#if defined _MSC_VER
#define ROR(W,i) _lrotr(W, i)
#define ROL(W,i) _lrotl(W, i)
#else
#define ROR(W,i) (((W)>>(i)) | ((W)<<(32-(i))))
#define ROL(W,i) (((W)<<(i)) | ((W)>>(32-(i))))
#endif

#define u32_in(x)            (*(u32*)(x))
#define u32_out(x, v)        {*((u32*)(x)) = (v);}

void Show_jun(u32 *src, int word_len);

void Show_junRndKeys(u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN]);
void LEA_Test();
void LEA_Keyschedule(u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN],const u8 pbKey[LEA_KEY_BYTE_LEN]);
void LEA_EncryptBlk(u8 pbDst[LEA_BLK_BYTE_LEN], const u8 pbSrc[LEA_BLK_BYTE_LEN],const u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN]);
void LEA_DecryptBlk(u8 pbDst[LEA_BLK_BYTE_LEN], const u8 pbSrc[LEA_BLK_BYTE_LEN], const u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN]);

void send_ble(unsigned char *data, int size);



//라운드키
  u32 rndkeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN] = {0x0,};
  u8 src1[LEA_BLK_BYTE_LEN] ={ 0x0, } ;
  u8 src2[LEA_BLK_BYTE_LEN] = { 0x0, };
  u8 src3[LEA_BLK_BYTE_LEN] = { 0x0, };

 u8 key[32] = {  
    0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0,
    0xf0, 0xe1, 0xd2, 0xc3, 0xb4, 0xa5, 0x96, 0x87, 0x78, 0x69, 0x5a, 0x4b, 0x3c, 0x2d, 0x1e, 0x0f 
  };




int decipherLen=0;
void setup()
{
 // delay(1000);
  Serial.begin(9600);
  hm10.begin(9600);
    LEA_Keyschedule(rndkeys, key);
 // Serial.write("\nTest Start\n");
}

void loop()
{



  
  u8 real_data[60];

if(Serial.available())
{
  char input;
  u8 text[16]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0xa0};
  u8 crypto[40];
  u8 send_data_to_phone[100];
  input=Serial.read();
  if(input=='a')
  {

LEA_EncryptBlk(crypto, text,rndkeys);
send_data_to_phone[0]=0xf3;
send_data_to_phone[1]=0x12;
send_data_to_phone[2]=16;
//sizeof는 안먹힘




memcpy(send_data_to_phone+3,crypto,16);


Serial.println("to_phonedata");
for(int aaa=0;aaa<19;aaa++)
{
Serial.print(send_data_to_phone[aaa],HEX);
  
}


Serial.println();
send_ble(send_data_to_phone,19);



  
}
}
  
  if(hm10.available())
  {
  unsigned char temp[2];
  delay(30);
 temp[0]=(unsigned char)hm10.read();
   
   temp[1]=(unsigned char)hm10.read();
   
   Serial.print("syn data:");
  
  Serial.print(temp[0],HEX);
  Serial.print(temp[1],HEX); 
  Serial.println();
  

if(temp[0]==0xf3&&temp[1]==0x12)
{
  char receive_size=0;
  char received_size=0;
  delay(30);
  receive_size=(unsigned char)hm10.read();
  Serial.print("receive_size:");
  Serial.println(receive_size,DEC);
  while(received_size<receive_size)
  {
      if(hm10.available()){
      real_data[received_size]=(unsigned char)hm10.read();
     
      received_size++;
      }
  }

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



}

 
  }
  
}

void LEA_Keyschedule(u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN],const u8 pbKey[LEA_KEY_BYTE_LEN])
{

#if defined(_LEA128_)
  
  u32 delta[4]= {0xc3efe9db, 0x44626b02, 0x79e27c8a, 0x78df30ec};
  u32 T[4] = {0x0,};

  T[0] = u32_in(pbKey);
  T[1] = u32_in(pbKey + 4);
  T[2] = u32_in(pbKey + 8);
  T[3] = u32_in(pbKey + 12);

  for(int i=0; i<LEA_NUM_RNDS; i++) 
  {
    T[0] = ROL(T[0] + ROL(delta[i&3], i), 1);
    T[1] = ROL(T[1] + ROL(delta[i&3], i+1), 3);
    T[2] = ROL(T[2] + ROL(delta[i&3], i+2), 6);
    T[3] = ROL(T[3] + ROL(delta[i&3], i+3), 11);

    pdRndKeys[i][0] = T[0];
    pdRndKeys[i][1] = T[1];
    pdRndKeys[i][2] = T[2];
    pdRndKeys[i][3] = T[1];
    pdRndKeys[i][4] = T[3];
    pdRndKeys[i][5] = T[1];
  }
#endif

#if defined(_LEA192_)

  u32 delta[6] = { 0xc3efe9db, 0x44626b02, 0x79e27c8a, 0x78df30ec, 0x715ea49e, 0xc785da0a};
  u32 T[6] = {0x0,};

  T[0] = u32_in(pbKey);
  T[1] = u32_in(pbKey + 4);
  T[2] = u32_in(pbKey + 8);
  T[3] = u32_in(pbKey + 12);
  T[4] = u32_in(pbKey + 16);
  T[5] = u32_in(pbKey + 20);

  for(int i=0; i<LEA_NUM_RNDS; i++) 
  {
    T[0] = ROL(T[0] + ROL(delta[i%6], i&0x1f), 1);
    T[1] = ROL(T[1] + ROL(delta[i%6], (i+1)&0x1f), 3);
    T[2] = ROL(T[2] + ROL(delta[i%6], (i+2)&0x1f), 6);
    T[3] = ROL(T[3] + ROL(delta[i%6], (i+3)&0x1f), 11);
    T[4] = ROL(T[4] + ROL(delta[i%6], (i+4)&0x1f), 13);
    T[5] = ROL(T[5] + ROL(delta[i%6], (i+5)&0x1f), 17);

    pdRndKeys[i][0] = T[0];
    pdRndKeys[i][1] = T[1];
    pdRndKeys[i][2] = T[2];
    pdRndKeys[i][3] = T[3];
    pdRndKeys[i][4] = T[4];
    pdRndKeys[i][5] = T[5];
  }

#endif

#if defined(_LEA256_)

  u32 delta[8] = {0xc3efe9db, 0x44626b02, 0x79e27c8a, 0x78df30ec, 0x715ea49e, 0xc785da0a, 0xe04ef22a, 0xe5c40957};
  u32 T[8] = {0x0,};

  T[0] = u32_in(pbKey);
  T[1] = u32_in(pbKey + 4);
  T[2] = u32_in(pbKey + 8);
  T[3] = u32_in(pbKey + 12);
  T[4] = u32_in(pbKey + 16);
  T[5] = u32_in(pbKey + 20);
  T[6] = u32_in(pbKey + 24);
  T[7] = u32_in(pbKey + 28);

  for(int i=0; i<LEA_NUM_RNDS; i++)
  {
    T[(6*i)&7] = ROL(T[(6*i)&7] + ROL(delta[i&7], i&0x1f), 1);
    T[(6*i + 1)&7] = ROL(T[(6*i + 1)&7] + ROL(delta[i&7], (i+1)&0x1f), 3);
    T[(6*i + 2)&7] = ROL(T[(6*i + 2)&7] + ROL(delta[i&7], (i+2)&0x1f), 6);
    T[(6*i + 3)&7] = ROL(T[(6*i + 3)&7] + ROL(delta[i&7], (i+3)&0x1f), 11);
    T[(6*i + 4)&7] = ROL(T[(6*i + 4)&7] + ROL(delta[i&7], (i+4)&0x1f), 13);
    T[(6*i + 5)&7] = ROL(T[(6*i + 5)&7] + ROL(delta[i&7], (i+5)&0x1f), 17);

    pdRndKeys[i][0] = T[(6*i)&7];
    pdRndKeys[i][1] = T[(6*i+1)&7];
    pdRndKeys[i][2] = T[(6*i+2)&7];
    pdRndKeys[i][3] = T[(6*i+3)&7];
    pdRndKeys[i][4] = T[(6*i+4)&7];
    pdRndKeys[i][5] = T[(6*i+5)&7];
  }
#endif

}


void LEA_EncryptBlk(u8 pbDst[LEA_BLK_BYTE_LEN], const u8 pbSrc[LEA_BLK_BYTE_LEN],const u32 pdRndKeys[LEA_NUM_RNDS][LEA_RNDKEY_WORD_LEN])
{
  u32 X0,X1,X2,X3;
  u32 temp;

  X0 = u32_in(pbSrc);
  X1 = u32_in(pbSrc + 4);
  X2 = u32_in(pbSrc + 8);
  X3 = u32_in(pbSrc + 12);

  for(int i=0; i<LEA_NUM_RNDS; i++)
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
  u32 X0,X1,X2,X3;
  u32 temp;

  X0 = u32_in(pbSrc);
  X1 = u32_in(pbSrc + 4);
  X2 = u32_in(pbSrc + 8);
  X3 = u32_in(pbSrc + 12);

  
  for(int i=0; i<LEA_NUM_RNDS; i++)
  {
    temp = X3;
    X3 = X2;
    X2 = X1;
    X1 = X0;
    X0 = temp;

    X1 = (ROR(X1,9) - (X0 ^ pdRndKeys[LEA_NUM_RNDS-1-i][0])) ^ pdRndKeys[LEA_NUM_RNDS-1-i][1];
    X2 = (ROL(X2,5) - (X1 ^ pdRndKeys[LEA_NUM_RNDS-1-i][2])) ^ pdRndKeys[LEA_NUM_RNDS-1-i][3];
    X3 = (ROL(X3,3) - (X2 ^ pdRndKeys[LEA_NUM_RNDS-1-i][4])) ^ pdRndKeys[LEA_NUM_RNDS-1-i][5];

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

void send_ble(unsigned char *data, int size)
{
 
 hm10.write(data,size);

Serial.print("send data:");
 Serial.write(data,size);
 for(int i=0;i<size;i++){
 Serial.print(data[i],HEX);
Serial.print("  ");
 }
}

