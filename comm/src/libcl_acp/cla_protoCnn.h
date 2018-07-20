#pragma once

#include "cla_proto.h"
#include "cla_mempool.h"
#include "cl_synchro.h"
#include "cl_speaker.h"

//*******************************************
//proxy_send --- cla_addr to,data
//proxy_recv --- cla_addr from,data

//*******************************************
//hole -- uint32 cnnid ; ��stun proxy,ֱ������ԴIPPORT����NAT��
//hole_ack -- uint32 cnnid ; ��stun proxy
//nat

//*******************************************
//cnn_proxy ��stun proxy 
typedef struct tag_cla_ptl_cnn_proxy
{
	//��ʶ
	uint32		connid;
	cla_addr	src;		//��ip portΪ����˴�proxy���.
	cla_addr	proxy;
}cla_ptl_cnn_proxy_t;
//cnn_proxy_ack -- uint32 cnnid ; ��proxy �ظ�


#define CLA_PTL_CNN_HEAD_LENGTH 9
#define CLA_PTL_PROXY_HEAD_LENGTH 7
#define CLA_PTL_CNN_DATA_HEAD_LENGTH 14  //

////���ݰ���ͷ�ܳ���(��ǰ��cnn_head)
////pcr : Parity check redundancy ��żУ������
////�ݲ�����֧�����ഫ�䣨���ʵʱ���ԣ�
//typedef struct tag_cla_ptl_cnn_data_pcr
//{
//	uchar	pcr_num;	//ͬһ��У�������.0��ʾ��У��,����У���,
//	uint16	pcr_seq;	//ͬһ��У����ͬһ����.���ڴ�һ���ֹ��
//}cla_ptl_cnn_data_pcr_t;


typedef struct tag_cla_ptl_cnn_data
{
	uint32					seq;			//���к�,1��ʼ����.0��ʾУ���
	uchar					cycle_seq;		//�ٶ�ͳ�Ʊ��,һ��ͳ������ʹ��ͬһ�����,0��Ч		

	tag_cla_ptl_cnn_data(void)
		:seq(1)
		,cycle_seq(0)
	{}
}cla_ptl_cnn_data_t;

#define ACK_ARR_LEN 100

typedef struct tag_cla_ptl_cnn_data_ack
{
	uint32		ack_seq;		//���Ͷ˿ɹ��ƶ�ack������
	uint32		need_seq;		//δȷ�Ϻ�
	uint32		win_num;		//ָʾ�Է���need_seq�𻹿��Է��Ͷ��ٸ�block
	
	//�ٶ����ڷ���
	uchar		csp_seq;		//�ٶ�����,һ���ٶ������ڿ��ܻᷢ�ض��ͬһ�����ڵ�recv_speedB��speed_i�����ڱ�ʾ���ڵı仯���ý��ն˱�֤���ظ�����ͬ�ٶ����ڵ��ٶ�
	uchar		csp_rerecv_num;		//�յ��ظ��İ���
	uint16		csp_num;		//�յ�����
	uint32		csp_speedB;
	
	uchar		size;
	uint64		recv_utick[ACK_ARR_LEN];		//��¼���յ�����ʱ��ʱ�䣬�����ݲ���Э�鴫�䣬ֻ�ǵ����ڲ�����ʹ��
	uint32		seq_nums[ACK_ARR_LEN];		//���һ�λظ�ACK_ARR_LEN��ȷ�ϰ�,Լ�����һ����Ϊ������Ӧ��
	uint32		wait_us[ACK_ARR_LEN];  //�յ�ȷ�ϰ�����˶���΢��Żظ�ACK
	
	tag_cla_ptl_cnn_data_ack(void)
		:ack_seq(0)
		,need_seq(1)
		,win_num(0)

		,csp_seq(0)
		,csp_rerecv_num(0)
		,csp_num(0)
		,csp_speedB(0)

		,size(0){}
}cla_ptl_cnn_data_ack_t;


int operator << (cl_ptlstream& ps, const cla_ptl_cnn_proxy_t& inf);
int operator >> (cl_ptlstream& ps, cla_ptl_cnn_proxy_t& inf);

int operator << (cl_ptlstream& ps, const cla_ptl_cnn_data_t& inf);
int operator >> (cl_ptlstream& ps, cla_ptl_cnn_data_t& inf);

int operator << (cl_ptlstream& ps, const cla_ptl_cnn_data_ack_t& inf);
int operator >> (cl_ptlstream& ps, cla_ptl_cnn_data_ack_t& inf);


//********************************************************************


//cla_channeli: connector���ڵǼ�ע��,���ҵ���
class cla_channeli;
class cla_connector;
class cla_channelListener
{
public:
	virtual ~cla_channelListener(void){}

	template<int I>
	struct S{enum{T=I};};
	
	typedef S<1> Connected;
	typedef S<2> Disconnected;
	typedef S<3> Data;
	typedef S<4> Writable;

	virtual void on(Connected,cla_channeli* ch){}
	virtual void on(Disconnected,cla_channeli* ch){}
	virtual void on(Data,cla_channeli* ch,cla_memblock *b){}
	virtual void on(Writable,cla_channeli* ch){}
};


enum {CLA_DISCONNECTED=0,CLA_CONNECTING=1,CLA_CONNECTED=2};

class cla_channeli : public cl_caller<cla_channelListener>
{
	friend class cla_connector;
	friend class cla_sockpool;
public:
	cla_channeli(int idx) 
	{
		__idx = idx;
		m_s.sid = __idx;
		reset();
	}
	virtual ~cla_channeli(void){}
	virtual void reset()
	{
		m_state = CLA_DISCONNECTED;
		m_fd = INVALID_SOCKET;
		m_mtu = 0;
		m_proxyhead_len = 0;
		m_s.did = -1;
		memset(&m_nmy,0,sizeof(m_nmy));
		memset(&m_ndes,0,sizeof(m_ndes));
		memset(&m_addr,0,sizeof(m_addr));
		m_max_sndls_size = CLA_SOCK_SENDBUF/CLA_MEMBLOCK_SIZE; //����ÿ�������ݴ�С
		m_max_rcvls_size = CLA_SOCK_RECVBUF/CLA_MEMBLOCK_SIZE;
	}
public:
	//app call
	virtual int attach(cla_addr &to,cla_addr &proxy)=0;
	virtual int connect(cla_addr &to,cla_addr &proxy)=0;
	virtual int disconnect()=0;
	virtual int send(cla_memblock *b,bool more=false)=0; //0��ɹ�,b���������
	virtual void update_recv_win_num(int cache_block_num)=0; //cache_block_num��ʾsockpool��ʣ��������δ����

	virtual int					set_bandwidth(int size)=0; //���òο�����
	virtual int					get_sendspeed(cla_sendspeed_t& s)=0;
	virtual int					get_recvspeed(cla_recvspeed_t& s)=0;
	virtual int					get_sndls_size()=0;

	//connector call
	virtual int on_connected()=0;
	virtual int on_recv_data(cla_memblock* b)=0;
	virtual int on_recv_data_ack(cla_memblock* b)=0;
	
	//
	//virtual int send_to(char *buf,int len,ULONGLONG utick){ return ::sendto(m_fd,buf,len,0,(sockaddr*)&m_addr,sizeof(m_addr));}
	int idx() const {return __idx;}
	int mtu() const {return m_mtu;}
protected:
	GETSET(int,m_state,_state)
protected:
	SOCKET			m_fd;
	int				__idx;  //��connector��factory�е�����	
	int				m_mtu;//ȡ˫����Сֵ
	int				m_proxyhead_len; //��0��ʾ��ת����.����ʱҪ��proxyͷ
	cla_addr		m_nmy;	//�ҵ�IPPORT,cnnproxy����ʱʹ��,syn��syn_ackҲ����
	cla_addr		m_ndes;	//Ŀ��IP��ַ
	sockaddr_in		m_addr; //��������ת��ַ,������Ŀ��,����m_bproxy����
	cla_session_t	m_s;
	unsigned int	m_max_sndls_size; //����Ͷ���,����sockpool�еĶ���
	unsigned int	m_max_rcvls_size; //����Ͷ���,����sockpool�еĶ���
};


//********************************************************************
//channelFactory: ������Ӧacceptor ����.
class cla_cnnFactory
{
public:
	virtual ~cla_cnnFactory(void){}
public:
	//����һ���µ�channel ������cla_channelHandlerָ�룬��IP��PORT����UDPChannel,UDPChannel����Ҫ��ע��
	virtual cla_channeli* cla_attach_channel(cla_addr &to,cla_addr &proxy)=0;
};

