
#ifndef EASYBOT__EASYBOT_HPP_
#define EASYBOT__EASYBOT_HPP_

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "rclcpp/clock.hpp"
#include "rclcpp/time_source.hpp"

using namespace std::chrono_literals;

//////////////Node declaration ////////////////////////////
#define INITIAL_BGN() rclcpp::Node::SharedPtr Init(){
#define INITIAL_END() return shared_from_this();}
#define INITIAL() INITIAL_BGN() INITIAL_END() 

#define DeriveRCLNode_(Nod) \
	struct Nod : public rclcpp::Node{

#define NodSkeletonEnd() \
	};

#define NodSkeleton1_(Nod) \
	DeriveRCLNode_(Nod) \
	NodConstruct1(Nod) 

#define NodConstruct1(Nod) \
	Nod(const char* name) : Node(name){} INITIAL() 

#define DECLARE_TIMER(tm) \
	rclcpp::TimerBase::SharedPtr tm;\
	virtual void OnTimer##tm() = 0;
#define INIT_TIMER(tmr, interval)\
	tmr = create_wall_timer(1ms* interval,\
		[this]()->void{ \
			OnTimer##tmr(); \
		});

#define ON_TIMER_BGN_(tm) void OnTimer##tm() override ;
#define ON_TIMER_BGN(tm)\
	void OnTimer##tm(){ \
		PROMOTION_BGN("On timer "<< #tm)
#define ON_TIMER_BGNx(x,tm) \
	void x::OnTimer##tm(){\
		PROMOTION_BGN("On timer "<< #tm)

#define NodSkeleton2_(Nod,tmr)\
	DeriveRCLNode_(Nod)\
	NodConstruct2(Nod,tmr)

#define NodConstruct2(Nod,tmr) \
	Nod(const char* name,int interval) \
	: Node(name){ INIT_TIMER(tmr, interval) } \
	DECLARE_TIMER(tmr) INITIAL() 

#define DECLARE_TIMESOURCE(ts) std::shared_ptr<rclcpp::TimeSource> ts;

// warning: 
// INIT_TIMESOURCE can only be used after node constructor called!!!
// because timesource constructor need a shared_ptr of a rclcpp node , 
// in the rclcpp node constructor,
// using shared_from_this()  as a parameter may cause a crash !!!
#define INIT_TIMESOURCE(ts) \
	ts=std::make_shared<rclcpp::TimeSource>(shared_from_this());

#define DECLARE_CLOCK(clock) \
	rclcpp::Clock::SharedPtr clock;

#define INIT_CLOCK(clock,clock_opt) \
	clock = std::make_shared<rclcpp::Clock>(clock_opt);

#define NodSkeleton3_(Nod,tm, ts, clk) \
	DeriveRCLNode_(Nod) \
	NodConstruct3(Nod,tm,ts,clk) 

#define NodConstruct3(Nod,tm,ts,clk) \
	Nod(const char* name,int interval, rcl_clock_type_t clk_opt) \
	: Node(name){ \
		INIT_TIMER(tm, interval) \
		INIT_CLOCK(clk,clk_opt) \
	}\
	DECLARE_TIMER(tm)  \
	DECLARE_TIMESOURCE(ts)  \
	DECLARE_CLOCK(clk) \
	INITIAL_BGN() \
		INIT_TIMESOURCE(ts); \
		ts->attachClock(clk); \
	INITIAL_END() 

#define IMPL_NOD_(Nod) struct Impl_##Nod : public Nod {  typedef Nod super;
#define IMPL_NOD1_(Nod) IMPL_NOD_(Nod) \
	Impl_##Nod(const char* name) : Nod(name){}
#define IMPL_NOD2_(Nod) IMPL_NOD_(Nod) \
	Impl_##Nod(const char* name,int ms) : Nod(name,ms){}
#define IMPL_NOD3_(Nod) IMPL_NOD_(Nod) \
	Impl_##Nod(const char* name,int ms,rcl_clock_type_t clk_opt) \
	: Nod(name,ms,clk_opt){}

#define IMPL_INIT_BGN() INITIAL_BGN() this->super::Init();
#define IMPL_INIT_END() INITIAL_END() 
//#define Ex_(x) (get_name()+std::string("/") + std::string(x)).c_str()
#define Ex_(x) x
////////////////////////RCL Meta //////////////////////////////////////
#define RCL_VAR_DECLARE(Role,Ty, Nm ,tag) \
	rclcpp::Role<Ty>::SharedPtr tag##_##Nm;
#define RCL_VAR_INITIAL(Role,Ty, Nm ,tag) \
	RCL_VAR_INITIAL_EX(Role,Ty,Nm,tag,#Nm) 
#define RCL_VAR_INITIAL_EX(Role,Ty, Nm ,tag,xtraName) \
	tag##_##Nm = create_##Role<Ty>( Ex_(xtraName));

#define END() PROMOTION_DONE()}
////////////////////////for debug info print //////////////////////////
#define DECL_PROMOTION() std::ostringstream oss; 
#define ADD_PROMOTION(P) oss << P;
#define PROMOTION_BGN(P) DECL_PROMOTION() ADD_PROMOTION(P)
#define PROMOTION_DONE() RCLCPP_INFO(get_logger(), oss.str().c_str()); 
//////////////////////client////////////////////////////////////////////////
#define CLT_VAR_DECLARE_BASIC(Nm,Ty)  RCL_VAR_DECLARE(Client , Ty, Nm, clt)

#define CLT_VAR_DECLARE_EXTRAL(Nm,Ty) \
Ty::Request::SharedPtr req_##Nm;\
int DoRequest##Nm( std::chrono::seconds time_out = 3s ){ \
 	auto& clt = clt_##Nm ; auto& req = req_##Nm; \
	while (!clt->wait_for_service(time_out)) { \
		if (!rclcpp::ok()) return 1;\
	}\
	clt->async_send_request(req,\
	[this](std::shared_future<Ty::Response::SharedPtr> fr)\
	->void{OnResponse##Nm(fr.get()); });\
	return 0;\
}

#define CLT_VAR_DECLARE(Nm,Ty) \
		CLT_VAR_DECLARE_BASIC(Nm,Ty)  CLT_VAR_DECLARE_EXTRAL(Nm,Ty) 
#define CLT_VAR_INITIAL_EX(Nm,Ty,xtraName) \
		RCL_VAR_INITIAL_EX(client,Ty,Nm,clt,xtraName)\
		req_##Nm = std::make_shared<Ty::Request>();
#define CLT_VAR_INITIAL(Nm,Ty) CLT_VAR_INITIAL_EX(Nm,Ty,#Nm)

#define ON_RESPONSE(Nm,Ty)\
	virtual void OnResponse##Nm(std::shared_ptr<Ty::Response> rsp) = 0;

#define RESPONSE_IMPL_BGN_(Nm,Ty) \
	void OnResponse##Nm(std::shared_ptr<Ty::Response> response) override;

#define RESPONSE_IMPL_BGN(Nm,Ty) \
	void OnResponse##Nm(std::shared_ptr<Ty::Response> response){\
		PROMOTION_BGN("On Responsing of "<<#Nm)

#define RESPONSE_IMPL_BGNx(x,Nm,Ty) \
	void x::OnResponse##Nm(std::shared_ptr<Ty::Response> response){\
		PROMOTION_BGN("On Responsing of "<<#Nm)

#define CltDecl(N,T)    CLT_VAR_DECLARE(N,T) ON_RESPONSE(N,T)
#define CltVarInit(N,T) CLT_VAR_INITIAL(N,T)
#define CltVarInitEx(N,T,xtraName) CLT_VAR_INITIAL_EX(N,T,xtraName)
#define RequestBgn(N) { RequestBgnEx(N) 
#define RequestEnd(N)   RequestEndEx(N) }

#define RequestBgnEx(N) \
	auto& req = req_##N; PROMOTION_BGN("Sending request "<<#N)
#define RequestEndEx(N) \
	auto result = DoRequest##N(); (void)result;PROMOTION_DONE() 
#define RequestCall(N,...) \
	RequestBgn(N)\
		for(int _=0;_!=1;__VA_ARGS__) ++_;\
	RequestEnd(N)
//--------------------------service--------------------------
#define SRV_VAR_DECLARE(Nm,Ty) RCL_VAR_DECLARE(Service,  Ty, Nm , srv)

#define SRV_VAR_INITIAL(Nm,Ty,xtraName) \
	srv_##Nm = create_service<Ty>(Ex_(xtraName),\
	 [this](const std::shared_ptr<rmw_request_id_t> req_hdr,\
		const std::shared_ptr<Ty::Request>    req,\
		std::shared_ptr<Ty::Response>   rsp)\
        ->void{ (void) req_hdr; OnRequest##Nm(req,rsp) ; } );

#define ON_REQUEST(Nm,Ty) virtual void \
	OnRequest##Nm(\
		const std::shared_ptr<Ty::Request> req,\
		std::shared_ptr<Ty::Response> rsp\
	) = 0;
#define REQUEST_IMPL_BGN_(Nm,Ty) \
	void OnRequest##Nm( const std::shared_ptr<Ty::Request> req,\
                std::shared_ptr<Ty::Response> rsp)override;

#define REQUEST_IMPL_BGN(Nm,Ty) \
	void OnRequest##Nm( const std::shared_ptr<Ty::Request> req,\
                std::shared_ptr<Ty::Response> rsp) {\
			PROMOTION_BGN("On recieving request of " << #Nm)

#define REQUEST_IMPL_BGNx(x,Nm,Ty) \
	void x::OnRequest##Nm( const std::shared_ptr<Ty::Request> req,\
                std::shared_ptr<Ty::Response> rsp) {\
			PROMOTION_BGN("On recieving request of " << #Nm)

#define SrvDecl(N,T)    SRV_VAR_DECLARE(N,T) ON_REQUEST(N,T)
#define SrvVarInit(N,T) SRV_VAR_INITIAL(N,T,#N)
#define SrvVarInitEx(N,T,xtraName) SRV_VAR_INITIAL(N,T,xtraName)
//-----------------------publisher----------------------------------------
#define PUB_VAR_DECLARE_BASIC(Nm,Ty) \
	RCL_VAR_DECLARE(Publisher, Ty, Nm, pub)
#define PUB_VAR_DECLARE_XTRAL(Nm,Ty) \
	Ty msg_##Nm; \
	void pub_msg_##Nm(){ pub_##Nm->publish(msg_##Nm);}
#define PubDecl(N,T) \
	PUB_VAR_DECLARE_BASIC(N,T) PUB_VAR_DECLARE_XTRAL(N,T) 
#define PubVarInit(N,T) \
	RCL_VAR_INITIAL(publisher,T,N,pub)
#define PubVarInitEx(N,T,xtraName) \
	RCL_VAR_INITIAL_EX(publisher,T,N,pub,xtraName)
#define PubBgn(N){ \
	PROMOTION_BGN("On publishing "<< #N)\
	auto& msg = msg_##N; auto p = pub_##N; (void)msg;(void)p; 

#define PubEnd(N)\
		p->publish(msg);\
		PROMOTION_DONE()\
	}
#define PubTopic(N,...) \
	PubBgn(N) \
		for(int _=0;_!=1; __VA_ARGS__)_++;\
	PubEnd(N)
//------------------------------subscription------------------------------
#define SUB_VAR_DECLARE(Nm,Ty) \
	RCL_VAR_DECLARE(Subscription, Ty, Nm,  sub)

#define SUB_VAR_INITIAL(Nm,Ty,xtraName) \
	sub_##Nm=create_subscription<Ty> ( xtraName,\
		[this](Ty::UniquePtr m){ \
			OnSubscription_##Nm( std::move(m) );\
		});

#define ON_SUBSCRIPTION(Nm,Ty) \
	virtual void OnSubscription_##Nm(Ty::UniquePtr msg) = 0;

#define SUBSCRIPTION_IMPL_BGN_(Nm,Ty)\
	void OnSubscription_##Nm(Ty::UniquePtr msg)override;
#define SUBSCRIPTION_IMPL_BGN(Nm,Ty)\
	void OnSubscription_##Nm(Ty::UniquePtr msg){\
		PROMOTION_BGN("On Subscription "<< #Nm) (void)msg;

#define SUBSCRIPTION_IMPL_BGNx(x,Nm,Ty)\
	void x::OnSubscription_##Nm(Ty::UniquePtr msg){\
		PROMOTION_BGN("On Subscription "<< #Nm) (void)msg;

#define SubDecl(N,T)  SUB_VAR_DECLARE(N,T) ON_SUBSCRIPTION(N,T)
#define SubVarInit(N,T) SUB_VAR_INITIAL(N,T,#N)
#define SubVarInitEx(N,T,xtraName) SUB_VAR_INITIAL(N,T,xtraName)
//--------------------------ACTION-----------------------------------------
#define ACT_VAR_DECLARE(nm,role,ty,pre) \
	rclcpp_action::role< ty >::SharedPtr pre##_##nm;
#define ACT_VAR_CREATE_EX(nm,role,ty,pre,xName) \
    pre##_##nm =rclcpp_action::create_##role<ty>(shared_from_this(),xName);
#define ACT_VAR_CREATE(nm,role,ty,pre) ACT_VAR_CREATE_EX(nm,role,ty,pre,#nm)
//--------------------------ACTION    client-------------------------------
#define ACT_CLT_VAR_DECLARE(nm,ty ) \
	ACT_VAR_DECLARE(nm, Client, ty, act_clt)
#define ACT_CLT_VAR_CREATE_EX(nm,ty,xName) \
	ACT_VAR_CREATE_EX( nm, client, ty, act_clt,xName)
#define ACT_CLT_VAR_CREATE(nm,ty) \
	ACT_CLT_VAR_CREATE_EX(nm,ty,#nm)
//--------------------------ACTION    server-------------------------------
#define ACT_SRV_VAR_DECLARE(nm,ty ) ACT_VAR_DECLARE(nm, Server, ty, act_svr)
#define ACT_SRV_VAR_CREATE(nm,ty )  ACT_VAR_CREATE( nm, server, ty, act_svr)
//--------------------------Parameters ------------------------------------
#define HOOK_UP_PARAMETER_EVENT_PROCESS() \
	register_param_change_callback( \
		[this](std::vector<rclcpp::Parameter> parameters)\
		-> rcl_interfaces::msg::SetParametersResult\
		{ return OnParaEvt( parameters );});
#define ON_PARA_EVT_BGN() \
	rcl_interfaces::msg::SetParametersResult \
		OnParaEvt(std::vector<rclcpp::Parameter> parameters) {\
			auto ret = rcl_interfaces::msg::SetParametersResult();\
			ret.successful = true;

#define ON_PARA_EVT_END()  return ret; }

class rcl_cpp_env{
	rcl_cpp_env( int argc, char * argv[] ){
		rclcpp::init(argc, argv); 
		exec = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
	};
	~rcl_cpp_env(){ rclcpp::shutdown(); }
	std::shared_ptr<rclcpp::executors::SingleThreadedExecutor> exec;
	std::vector<rclcpp::Node::SharedPtr> nodes;
public:
	static  rcl_cpp_env* getInstance( int argc, char * argv[] ) {
		static rcl_cpp_env local( argc,argv );
		return &local;
	}
	rcl_cpp_env* add_node( rclcpp::Node::SharedPtr nod ){
		exec->add_node(nod);
		nodes.push_back(nod);
		return this;
	}
	template <class T, class... Args>
	rcl_cpp_env*  create(Args&&... args){
		return add_node(std::make_shared<T>(args...)->Init()); 
	}
	int run() { exec->spin(); return 0; }
	template<typename T>
	int run_until_future_done(std::shared_future< T > &action ){
		exec->spin_until_future_complete( action );
		return 0;
	}
};
#define ROS_Bgn() \
int main(int argc ,char* argv[]){\
        rcl_cpp_env* env = rcl_cpp_env::getInstance(argc,argv);
#define NodNew(T, ...) env->create<T>(__VA_ARGS__);

#define ROS_End() \
	return env->run();\
}
#define SingleNode(T,...) \
	ROS_Bgn() \
		env->create<T>(__VA_ARGS__);\
	ROS_End()

#define NPubDecl(N,T) \
	std::vector< rclcpp::Publisher<T>::SharedPtr > N;\
	T msg_##N;
#define NPubVarInit(N,T, num)\
        N.clear();\
        for(Gen g(#N); g.i<num;) \
		N.push_back(create_publisher<T>( g() ));
#define NPubBgn(N,i) \
	{\
		 auto p = N[i]; auto& msg = msg_##N;
#define NPubEnd() \
		p->publish(msg);\
	}
struct Gen{
        int i=0;
        std::string _1;
        Gen(const char* _1_):_1(_1_){}
        std::string operator()() { 
		return std::string("/") +_1 + "/"+std::to_string(i++);
	}
};
#define RET_IF(cond, act) if (cond) { return act; }
#define xRET_IF(cond,ret, ...) \
	if(cond){\
		for(int _=0;_!=1;__VA_ARGS__)_++;\
		return ret;\
	}
#define RET_IF_ELSE(cond,act,act2) \
	if(cond){ \
		return act;\
	}\
	return act2;
#define NOOP (void)0

#define ENUM_CLS(A,...) enum class A { __VA_ARGS__};
#define ENUM_CLS_Bgn(A,...) enum class A { __VA_ARGS__}; \
	struct A##Map { \
		static std::map<A,const char*> theMap;\
		static const char* ToName(A k) { \
			auto i = theMap.find(k);\
			if(i!=theMap.end()) return i->second; \
			return NULL;\
		}\
		static void Init(){

#define ENUM_CLS_End(A)\
		}\
		static A##Map* getInstance(){ static A##Map x; return &x;}\
		private: A##Map(){Init();}\
	};\
	std::map<A,const char*> A##Map::theMap;\
	A##Map* var_##A##Map = A##Map::getInstance(); \
	std::string Enum2Str(A x){ return std::string(A##Map::ToName(x)) ; }

#define TO_MAP(S,e) S##Map::theMap[S::e]=#e;
#define TO_BOOL(S,e)\
	bool is_##e(S v) { return v==S::e;} \
	void set_##e(S& v){v=S::e;}

#endif  // EASYBOT__EASYBOT_HPP_
