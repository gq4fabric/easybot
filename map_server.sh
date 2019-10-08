INLINE=0 SHOW_CODE=1 SAVE_CODE=0 COLCON=1
req="easybot rclcpp nav_msgs"
includes=("easybot/easybot.hpp"\
	  "rclcpp/clock.hpp"\
	  "rclcpp/time_source.hpp")
opts=("TIMER,tm,10" "TIMESOURCE,ts,clock" \
   "Pub,map,nav_msgs::msg::OccupancyGrid,nav_msgs/msg/occupancy_grid.hpp")
myImpl="decl cons timer_tm glob sub_image"

source ezb.sh&& mk_all $0

##########################################################################################
#glob|//global codes here ...
#glob|
##########################################################################################
#decl|void InitMap(const char*nm="world",float res = .1f, int w = 100, int h = 100  ){
#decl|	auto* msg = &msg_map; auto & info = msg->info; auto& pos = info.origin.position;
#decl|	auto& orient = msg->info.origin.orientation; 
#decl|	msg->header.frame_id = nm;
#decl|	info.resolution = res; info.width = w; info.height = h;
#decl|	pos.x =  pos.y = -(w*h) / 2; pos.z = 0;
#decl|	orient.x = orient.y = orient.z = 0; orient.w = 1;
#decl|	for (size_t i = 0; i < w*h; ++i) msg->data.push_back(-1);
#decl|}
##########################################################################################
#cons|// in constructor code here...
#cons|InitMap();
##########################################################################################
#timer_tm|//timer code here...
#timer_tm|PubBgn(map) 
#timer_tm| 	static int lhs = 0, center = 1, rhs = 2;
#timer_tm| 	int val[6]={-1,-1,-1,0,100,0};
#timer_tm| 	int idx[6]={lhs,center,rhs,++lhs,++center,++rhs};
#timer_tm| 	int area = msg.info.width * msg.info.height;
#timer_tm| 	for (int i = 0;i<6;++i) msg.data[ idx[i]%area ] = val[i];
#timer_tm| 	msg.header.stamp = clock->now();
#timer_tm|PubEnd(map)
##########################################################################################
#sub_image|//on sub img ..
##########################################################################################
#req_xxx|
#rsp_yyy|
