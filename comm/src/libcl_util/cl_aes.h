#pragma once
//enum KeySize { Bits128, Bits192, Bits256 };  // key size, in bits, for construtor
typedef unsigned char   u1byte; /* an 8 bit unsigned character type */
typedef unsigned short  u2byte; /* a 16 bit unsigned integer type   */
typedef unsigned int	u4byte; /* a 32 bit unsigned integer type   */
typedef signed char     s1byte; /* an 8 bit signed character type   */
typedef signed short    s2byte; /* a 16 bit signed integer type     */
typedef signed int		s4byte; /* a 32 bit signed integer type     */
/* 2. Standard interface for AES cryptographic routines             */
/* These are all based on 32 bit unsigned values and may require    */
/* endian conversion for big-endian architectures                   */
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN
#endif

/* 3. Basic macros for speeding up generic operations               */
/* Circular rotate of 32 bit values                                 */

#define rotr(x,n)   (((x) >> ((int)(n))) | ((x) << (32 - (int)(n))))
#define rotl(x,n)   (((x) << ((int)(n))) | ((x) >> (32 - (int)(n))))

/* Invert byte order in a 32 bit variable                           */
#define bswap(x)    (rotl(x, 8) & 0x00ff00ff | rotr(x, 8) & 0xff00ff00)
/* Extract byte from a 32 bit quantity (little endian notation)     */
#define byte(x,n)   ((u1byte)((x) >> (8 * n)))
/* Input or output a 32 bit word in machine order     */
#ifdef LITTLE_ENDIAN
#define u4byte_in(x)  (*(u4byte*)(x))
#define u4byte_out(x, v) (*(u4byte*)(x) = (v))
#else
#define u4byte_in(x)  bswap(*(u4byte)(x))
#define u4byte_out(x, v) (*(u4byte*)(x) = bswap(v))
#endif

//AES���������ұ�׼�����о���NISTּ��ȡ��DES��21���͵ļ��ܱ�׼
//Ӧ���ڴ��������������ļ������ܺ��Сһ��

/********************************
ʹ��˵��:
set_key(key,key_len=key��λ��).
��key = 32�ֽڵĳ���ʱ, key_len = 256.
key����֧��16byte=128,24byte,32byte=256..
����/���� ʱÿ16�ֽ�Ϊ1����Ԫ.
*********************************/
class cl_aes 
{
public:
	cl_aes(){}
	virtual ~cl_aes(){}
public:
	void set_key(const u1byte key[], const u4byte key_len);
	void encrypt(const u1byte in_blk[16], u1byte out_blk[16]);
	void decrypt(const u1byte in_blk[16], u1byte out_blk[16]);

	//�������ֲ�����,ֱ�ӽ����ݼӵ�ԭ��buf
	void encrypt_n(u1byte iobuf[],int size);
	void decrypt_n(u1byte iobuf[],int size);
	
	//�������key,һ��Ϊ16B����32B.setʱҪתΪbit.
	static int rand_key(u1byte key[],int size);
private:
	u4byte  k_len;
	u4byte  e_key[64];
	u4byte  d_key[64];
};
