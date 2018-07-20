#pragma once
#include "cla_timer.h"
#include "cla_protoCnn.h"
#include "cla_acceptor.h"
#include "cla_stunClient.h"
#include "cla_singleton.h"

//�ں�sock ״̬.
enum ECLA_CORE_STATE
{
	ECLA_CORE_IDLE=0,
	ECLA_CORE_MYADDRING,
	ECLA_CORE_CNNPROXYING,
	ECLA_CORE_CONNECTING,
	ECLA_CORE_ACCEPTING,
	ECLA_CORE_CONNECTED,
	ECLA_CORE_ACCEPTED,
};


//˵��; connector �� �������� �� cnnFactory ���������ݶ�Ӧ. cnnFactory��������ch��״̬.
class cla_connector : public cla_timerHandler, public cla_acceptorHandler
{
public:
	cla_connector();
	virtual ~cla_connector(void);

	
	typedef struct tag_cnnSession
	{
		bool			is_used;
		int				state;
		unsigned char	des_nattype; //Ŀ���nattype����
		unsigned char	des_nattype2; //��ʱ��¼�������ж��Ƿ���Ҫ��req_hole��
		cla_channeli*	handle;
		DWORD			last_send_tick;
		DWORD			last_recv_tick;
		DWORD			begin_tick;

		void reset()
		{
			is_used = false;
			state = ECLA_CORE_IDLE;
			des_nattype = 0;
			des_nattype2 = 0;
			handle = 0;
			last_send_tick = 0;
			last_recv_tick = 0;
			begin_tick = (DWORD)-60000; //���60�����ڲ�ʹ��

		}
		tag_cnnSession(void){reset();}
	}UDPSession_t;
public:
	int		init(unsigned short port,cla_cnnFactory* cnnf,const char* stunsvr);
	void	fini();
	int		handle_select_read(unsigned long delay_usec=0) {return m_apt.handle_select_read(delay_usec);}
	void	set_mtu(int mtu) {if(mtu<100) mtu=100;if(mtu>=CLA_MEMBLOCK_SIZE) mtu=CLA_MEMBLOCK_SIZE; m_mtu=mtu;}
	int		get_socknum()const {return m_socknum;}

	virtual void on_timer(int e);
	//channel call
	int connect(cla_channeli* ch,int nattype);
	int disconnect(cla_channeli* ch);

	//acceptor call
	//����1��ʾ block �Ӻ���.
	virtual int on_acceptor_recv(cla_memblock *block,sockaddr_in& addr);
private:
	//register
	int register_channel(cla_channeli *ch,int nattype);
	void unregister_channel(cla_channeli *ch);

	//ptl send
	void ptl_send_channel_buf(int i,char* buf,int len);

	
	template<typename T>
	void ptl_send_T1(cla_cmd_t cmd,const T& inf,const sockaddr_in& to_addr)
	{
		_tmp_sps.attach(_tmp_sbuf,512,0);
		_tmp_sps << cmd;
		_tmp_sps << inf;
		::sendto(m_fd,_tmp_sbuf,_tmp_sps.length(),0,(const sockaddr*)&to_addr,sizeof(sockaddr_in));
	}
	template<typename T>
	void ptl_send_proxy_T1(cla_cmd_t cmd,const T& inf,const cla_addr& ndes,const sockaddr_in& proxy_addr)
	{
		_tmp_sps.attach(_tmp_sbuf,512,0);
		_tmp_sps << CLA_PTL_PROXY_SEND;
		_tmp_sps << ndes;
		_tmp_sps << cmd;
		_tmp_sps << inf;
		::sendto(m_fd,_tmp_sbuf,_tmp_sps.length(),0,(const sockaddr*)&proxy_addr,sizeof(sockaddr_in));
	}

	void ptl_send_nat(int i,cla_addr& ndes);
	void ptl_send_cnnproxy(int i);

	void ptl_send_cnn_cmd(int i,cla_cmd_t cmd); //ֻ���� cmm+session
	void ptl_send_cnn_syn(int i,cla_cmd_t cmd); //��mtu��ndes
	void ptl_send_cnn_cls(cla_session_t& s,cla_addr& ndes,cla_addr& nproxy);

	//
	void on_timer_send();
	bool check_session_ok(cla_session_t& s,cla_addr& ndes);
	void check_update_port(int i,unsigned short nport);
	int on_recv(cl_ptlstream& ps,cla_addr& from,cla_addr& proxy,cla_memblock* b);
private:
	bool				m_binit;
	cla_cnnFactory		*m_cnnf; //���յ���������ʱcall new channel
	cla_acceptor		m_apt;
	cla_stunClient		m_stunc;
	UDPSession_t		**m_udps;
	int					m_socknum; //����������
	int					m_max_i;//�����Ч�±�.������������ʱ���Ǵ�ͷ�ҿ��е�.
	DWORD				m_tick; //����
	uint32				m_mtu;	//���ְ�ֵ,��ͷ
	//*********************
	//��ʱ����
	sockaddr_in				_tmp_addr; //����hole��͸������ת����ʱ��
	uchar					_tmp_cmd;
	char					_tmp_sbuf[512];
	char					*_tmp_ts_sbuf;
	cla_session_t			_tmp_ss;
	cl_ptlstream			_tmp_sps;
	cla_session_t			_tmp_rs;
	cl_ptlstream			_tmp_rps;
};

//typedef cla_singleton<cla_connector> cla_connectorSngl;
