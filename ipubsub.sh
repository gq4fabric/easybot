INLINE=0 SHOW_CODE=1 SAVE_CODE=0 COLCON=1 RUNEXE=0

req="easybot rclcpp sensor_msgs OpenCV" 
includes=("easybot/easybot.hpp"\
	  "opencv2/opencv.hpp"\
	  "opencv2/imgproc/imgproc.hpp"\
	  "opencv2/highgui/highgui.hpp")
opts=("TIMER,tm,80"\
      "TIMESOURCE,ts,clk" \
      "Sub,image,sensor_msgs::msg::Image,sensor_msgs/msg/image.hpp" \
      "Pub,image,sensor_msgs::msg::Image,sensor_msgs/msg/image.hpp")
myImpl="decl cons timer_tm glob sub_image"

source ezb.sh && mk_all $0 

##########################################################################################
#decl|int nFrms=0;
#decl|cv::VideoCapture cap;
#decl|size_t width=320, height=240;
#decl|cv::Mat f;
#decl|cv::CascadeClassifier faceCascader;
##########################################################################################
#cons|cap.open(0);
#cons|cap.set(CV_CAP_PROP_FRAME_WIDTH, static_cast<double>(width));
#cons|cap.set(CV_CAP_PROP_FRAME_HEIGHT, static_cast<double>(height));
#cons|faceCascader.load("/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml");
##########################################################################################
#glob|std::string mat_type2encoding(int mat_type) {
#glob|#define _(a,b) RET_IF(mat_type==a,b)
#glob|	_(CV_8UC1, "mono8") _(CV_8UC3, "bgr8") _(CV_16SC1, "mono16") _(CV_8UC4, "rgba8")
#glob|	throw std::runtime_error("Unsupported encoding type");
#glob|}
#glob|
#glob|void convert( const cv::Mat & frame, size_t frame_id, sensor_msgs::msg::Image & msg) {
#glob|  msg.height = frame.rows;
#glob|  msg.width = frame.cols;
#glob|  msg.encoding = mat_type2encoding(frame.type());
#glob|  msg.step = static_cast<sensor_msgs::msg::Image::_step_type>(frame.step);
#glob|  size_t size = frame.step * frame.rows;
#glob|  msg.data.resize(size);
#glob|  memcpy(&msg.data[0], frame.data, size);
#glob|  msg.header.frame_id = std::to_string(frame_id);
#glob|}
##########################################################################################
#glob|int encoding2mat_type(const std::string & encoding){
#glob|#define _(a,b) RET_IF(encoding==a,b)
#glob|	_("mono8",CV_8UC1) _("bgr8",CV_8UC3)   _("mono16",CV_16SC1) _("rgba8",CV_8UC4) _("32FC1",CV_32FC1) _("rgb8",CV_8UC3)
#glob|  throw std::runtime_error("Unsupported encoding type");
#glob|}
##########################################################################################
#timer_tm| cap >> f;
#timer_tm| RET_IF(f.empty(),NOOP);
#timer_tm| PubTopic(image, convert(f,nFrms++,msg),oss<<nFrms)
##########################################################################################
#sub_image|static int nFrms=0;
#sub_image|++nFrms;
#sub_image|ADD_PROMOTION(nFrms << " Frame")
#sub_image|cv::Mat frame( msg->height, msg->width, encoding2mat_type(msg->encoding),
#sub_image|const_cast<unsigned char *>(msg->data.data()), msg->step);
#sub_image|cv::Mat gray, faceROI;
#sub_image|std::vector<cv::Rect> faces, eyes;
#sub_image|cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY); 
#sub_image|
#sub_image|cv::equalizeHist(gray, gray);
#sub_image|faceCascader.detectMultiScale(gray, faces,1.2, 3, 0, cv::Size(100, 100));
#sub_image|
#sub_image|for(auto f:faces) cv::rectangle(frame,f,cv::Scalar(0,255,0), 1, 8, 0);
#sub_image|		
#sub_image|cv::Mat cvframe = frame; 
#sub_image|if (msg->encoding == "rgb8")  
#sub_image|	cv::cvtColor(frame, frame, cv::COLOR_RGB2BGR); 
#sub_image|cv::Mat cvframe2 = frame; 
#sub_image|cv::imshow("face", cvframe);
#sub_image|cv::imshow("gray", gray);
#sub_image|cv::waitKey(1);
##########################################################################################
#req_xxx|
#rsp_yyy|
