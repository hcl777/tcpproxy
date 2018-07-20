#pragma once

#include "cla_proto.h"
#include "cla_mempool.h"
#include "cl_synchro.h"
#include "cl_speaker.h"

//*******************************************
//proxy_send --- cla_addr to,data
//proxy_recv --- cla_addr from,data

//*******************************************
//hole -- uint32 cnnid ; 走stun proxy,直接向来源IPPORT发起NAT包
//hole_ack -- uint32 cnnid ; 走stun proxy
//nat

//*******************************************
//cnn_proxy 走stun proxy 
typedef struct tag_cla_ptl_cnn_proxy
{
	//标识
	uint32		connid;
	cla_addr	src;		//此ip port为发起端从proxy获得.
	cla_addr	proxy;
}cla_ptl_cnn_proxy_t;
//cnn_proxy_ack -- uint32 cnnid ; 走proxy 回复


#define CLA_PTL_CNN_HEAD_LENGTH 9
#define CLA_PTL_PROXY_HEAD_LENGTH 7
#define CLA_PTL_CNN_DATA_HEAD_LENGTH 14  //

////数据包的头总长度(含前面cnn_head)
////pcr : Parity check redundancy 奇偶校验冗余
////暂不考虑支持冗余传输（提高实时线性）
//typedef struct tag_cla_ptl_cnn_data_pcr
//{
//	uchar	pcr_num;	//同一组校验包个数.0表示无校验,不含校验包,
//	uint16	pcr_seq;	//同一组校验用同一个号.周期大一点防止串
//}cla_ptl_cnn_data_pcr_t;


typedef struct tag_cla_ptl_cnn_data
{
	uint32					seq;			//序列号,1开始递增.0表示校验包
	uchar					cycle_seq;		//速度统计编号,一个统计周期使用同一个编号,0无效		

	tag_cla_ptl_cnn_data(void)
		:seq(1)
		,cycle_seq(0)
	{}
}cla_ptl_cnn_data_t;

#define ACK_ARR_LEN 100

typedef struct tag_cla_ptl_cnn_data_ack
{
	uint32		ack_seq;		//发送端可估计丢ack包总数
	uint32		need_seq;		//未确认号
	uint32		win_num;		//指示对方从need_seq起还可以发送多少个block
	
	//速度周期反馈
	uchar		csp_seq;		//速度周期,一个速度周期内可能会发关多次同一个周期的recv_speedB，speed_i仅用于表示周期的变化，让接收端保证不重复利用同速度周期的速度
	uchar		csp_rerecv_num;		//收到重复的包数
	uint16		csp_num;		//收到个数
	uint32		csp_speedB;
	
	uchar		size;
	uint64		recv_utick[ACK_ARR_LEN];		//记录接收到数据时的时间，此数据不用协议传输，只是单方内部程序使用
	uint32		seq_nums[ACK_ARR_LEN];		//最多一次回复ACK_ARR_LEN个确认包,约定最后一个包为立即回应包
	uint32		wait_us[ACK_ARR_LEN];  //收到确认包后等了多少微秒才回复ACK
	
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


//cla_channeli: connector用于登记注册,并且调度
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
		m_max_sndls_size = CLA_SOCK_SENDBUF/CLA_MEMBLOCK_SIZE; //不管每个包数据大小
		m_max_rcvls_size = CLA_SOCK_RECVBUF/CLA_MEMBLOCK_SIZE;
	}
public:
	//app call
	virtual int attach(cla_addr &to,cla_addr &proxy)=0;
	virtual int connect(cla_addr &to,cla_addr &proxy)=0;
	virtual int disconnect()=0;
	virtual int send(cla_memblock *b,bool more=false)=0; //0则成功,b被放入队列
	virtual void update_recv_win_num(int cache_block_num)=0; //cache_block_num表示sockpool还剩多少数据未被读

	virtual int					set_bandwidth(int size)=0; //设置参考带宽
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
	int				__idx;  //在connector和factory中的索引	
	int				m_mtu;//取双方最小值
	int				m_proxyhead_len; //非0表示中转发送.发送时要加proxy头
	cla_addr		m_nmy;	//我的IPPORT,cnnproxy连接时使用,syn和syn_ack也互发
	cla_addr		m_ndes;	//目标IP地址
	sockaddr_in		m_addr; //可能是中转地址,可能是目标,根据m_bproxy决定
	cla_session_t	m_s;
	unsigned int	m_max_sndls_size; //最大发送队列,包括sockpool中的队列
	unsigned int	m_max_rcvls_size; //最大发送队列,包括sockpool中的队列
};


//********************************************************************
//channelFactory: 用于响应acceptor 连接.
class cla_cnnFactory
{
public:
	virtual ~cla_cnnFactory(void){}
public:
	//创建一个新的channel 并返回cla_channelHandler指针，把IP，PORT赋给UDPChannel,UDPChannel不需要再注册
	virtual cla_channeli* cla_attach_channel(cla_addr &to,cla_addr &proxy)=0;
};

