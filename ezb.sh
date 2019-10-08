create='ros2 pkg create'
options='--build-type ament_cmake --maintainer-name "Qin Gao" --maintainer-email gaoqin20031001@163.com'
prefix=eb_
comments="// Add your codes here..."
_v=(" " "const char* name" "const char* name,int interval" "const char* name,int interval,rcl_clock_type_t clk_opt")
_z=(" " "name" "name, interval" "name, interval,clk_opt")
build_install(){
	sed 's/# find dependencies/set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")/g' $1/CMakeLists.txt > x
	mv x $1/CMakeLists.txt
	cd $1 ; colcon build ; source install/setup.bash; cd .. ; #&& clear
}
recreate(){ rm -rf $1; echo $create $1 $options | bash; }
mk_hpp_prj(){
	prj=$1 hpp=$2.hpp
	recreate $prj && cp $hpp $prj/include/$prj/$hpp
	sed 's/(ament_cmake REQUIRED)/(ament_cmake REQUIRED)\nament_export_include_directories(include)\ninstall(DIRECTORY include\/ DESTINATION include)/g' $prj/CMakeLists.txt>y
	mv y $prj/CMakeLists.txt && build_install $prj
}
mk_interface_prj(){
	prj=${prefix}$1 typ=$2 nms=$3
	recreate $prj && mkdir $prj/$typ
	for name in $nms; do t=$t\ \\\"$typ\\\/$name.$typ\\\" ; cp $name.$typ $prj/$typ/ ; done;
	sed s/_prj_/$prj/g package.xml.eb > $prj/package.xml
	sed s/_prj_/$prj/g CMakeLists.txt.eb |sed "s/__files__/$t/g" >$prj/CMakeLists.txt
	unset t && build_install $prj
}
mk_srv_prj(){ mk_interface_prj $1 srv "$2"; }
mk_msg_prj(){ mk_interface_prj $1 msg "$2"; }
mk_cpp_prj(){
	prj=${prefix}$1 ; rm -rf $prj;
	echo $create $prj --dependencies $2 $options --cpp-node-name $3 | bash
	cp $3.cpp $prj/src/ && build_install $prj
}
scan(){
	if [ $# -eq 1 ]; then NODE=$1 TYPE=1; fi
	if [ $# -eq 3 ]; then
		if [ $1 = "TIMER" ];      then TIMER=$2 INTERVAL=$3 TYPE=2; fi
		if [ $1 = "TIMESOURCE" ]; then TS=$2 CLK=$3 TYPE=3;         fi
	fi
}
incl(){ if [ $# -eq 4 ]; then echo "#include <${4}>";                   fi; }
decl(){ if [ $# -eq 4 ]; then echo -e "\t${1}Decl(${2},${3})" ;         fi; }
init(){ if [ $# -eq 4 ]; then echo -e "\t\t${1}VarInit(${2},${3})";     fi; }
implement_while_declare(){ echo -e "\t${1}(${2},${3})\n\t\t${comments}"; cat $cod | sed "s/^/\t/g"; echo -e "\tEND()"; }
implement_only(){ echo -e "${1}x(My${NODE},${2},${3})\n\t${comments}"; cat $cod | sed "s/^/\t/g"; echo 	"END()"; }
declare_only(){   echo -e "\t${1}_(${2},${3})"; }
timer_implement(){
	_t_bgn=("ON_TIMER_BGNx(My${NODE},${TIMER})\n\t${comments}" "\tON_TIMER_BGN(${TIMER})\n\t\t${comments}")
	_t_end=("END()" "\tEND()")
	echo -e ${_t_bgn[$INLINE]}
	if [ $INLINE -eq 1];then cat timer_${TIMER}|sed "s/^/\t/g"; else cat timer_${TIMER}|sed "s/^/\t\t/g";fi
	echo -e ${_t_end[$INLINE]} 
}
_implement() {
	if [ $1 = "TIMER" ];then timer_implement; fi
	f1=(implement_only implement_while_declare)
	if [ $# -eq 4 ]; then
		if [ $1 = "Sub" ]; then cod=sub_${2};`echo ${f1[$INLINE]}` SUBSCRIPTION_IMPL_BGN ${2} ${3}; fi
		if [ $1 = "Clt" ]; then cod=rsp_${2};`echo ${f1[$INLINE]}` RESPONSE_IMPL_BGN ${2} ${3}; fi
		if [ $1 = "Srv" ]; then cod=req_${2};`echo ${f1[$INLINE]}` REQUEST_IMPL_BGN ${2} ${3}; fi
	fi
}
exline_declare(){
	if [ $1 = "TIMER" ]; then echo -e "\tON_TIMER_BGN_(${TIMER})"; fi
	if [ $# -eq 4 ]; then
		if [ $1 = "Sub" ]; then declare_only SUBSCRIPTION_IMPL_BGN ${2} ${3}; fi
		if [ $1 = "Clt" ]; then declare_only RESPONSE_IMPL_BGN ${2} ${3}; fi
		if [ $1 = "Srv" ]; then declare_only REQUEST_IMPL_BGN ${2} ${3}; fi
	fi
}
implement_constructor(){
	_cons_bgn=("${_my}::${_my}(${_v[$TYPE]})\n\t : ${_imp}(${_z[$TYPE]}) {"  "\t${_my}(${_v[$TYPE]})\n\t : ${_imp}(${_z[$TYPE]}) {" )
	_cons_end=("}" "\t}")
	
	echo -e ${_cons_bgn[ $INLINE]}
	if [ $INLINE -eq 1];then cat cons |sed "s/^/\t/g"; else cat cons | sed "s/^/\t\t/g";fi
	echo -e ${_cons_end[ $INLINE]} 
}
external(){ for i in ${includes[@]} ; do echo "#include <$i>"; done;  cat glob; }
my_declares(){ cat decl | sed "s/^/\t/g"; }
declare_constructor(){ echo -e "\t${_my}(${_v[$TYPE]});"; }
abstract_node_bgn(){ echo -e "\nNodSkeleton${_x[$TYPE]}"; }
abstract_node_end(){ echo -e "NodSkeletonEnd()\n"; }
implement_nod_bgn(){ echo -e "IMPL_NOD${TYPE}_($NODE)\n\tIMPL_INIT_BGN()";}
implement_nod_end(){ echo -e "\tIMPL_INIT_END()\nNodSkeletonEnd()\n";}
my_node_bgn(){       echo -e "struct ${_my} : public ${_imp} {"; }
my_node_end(){       echo -e "};"; }
main(){   echo -e "\nSingleNode(${_xx[ $TYPE ]})\n"; }
ToCPP() {
	snippet_bgn
	unset NODE&&unset TYPE&& unset TIMER&& unset INTERVAL&& unset TS&&unset CLK
	for i in $*; do scan `echo $i | sed 's/,/ /g'`; done;
	_my=My${NODE} _imp=Impl_${NODE} _x=(" " "1_(${NODE})" "2_($NODE,$TIMER)" "3_($NODE,$TIMER,$TS,$CLK)")
	_xx=(" " "$_my,\"$_my\"" "$_my,\"$_my\",$INTERVAL" "$_my,\"$_my\",$INTERVAL,RCL_ROS_TIME" )
	for i in $*; do incl `echo $i | sed 's/,/ /g'`; done;
	external
	abstract_node_bgn; for i in $*; do decl `echo $i | sed 's/,/ /g'`; done; abstract_node_end
	implement_nod_bgn; for i in $*; do init `echo $i | sed 's/,/ /g'`; done; implement_nod_end
	my_node_bgn 
		my_declares
		f1=(declare_constructor implement_constructor) f2=(exline_declare _implement)
		`echo ${f1[$INLINE]}`;for i in $*; do `echo ${f2[$INLINE]}` `echo $i|sed 's/,/ /g'`;done;
	my_node_end
	f1=(implement_constructor noop) f2=(_implement noop)
	`echo ${f1[$INLINE]}`;for i in $*; do `echo ${f2[$INLINE]}` `echo $i|sed 's/,/ /g'`;done;
	snippet_end;  main
}
snippet_bgn(){ for i in $myImpl; do grep "^#$i" $0 |sed "s/#$i|//g" >$i;done; }
snippet_end(){ for i in $myImpl; do rm -rf $i; done; }
clarify(){ _x1=$1 _y1=${_x1#*/} _z1=${_y1%.*} my_prj=$_z1 my_cpp=$_z1 exe=./${prefix}$my_prj/build/${prefix}${my_prj}/$my_cpp;}
mk_all(){
	clarify $1 && unset nOpt
	for i in ${opts[@]} ;do nOpt="$nOpt $i"; done
	ToCPP $my_cpp $nOpt > $my_cpp.cpp
	if [ $SHOW_CODE -eq 1 ]; then cat $my_cpp.cpp; fi
	if [ $COLCON    -eq 1 ]; then mk_cpp_prj $my_prj "$req" $my_cpp  ; fi
	if [ $RUNEXE    -eq 1 ]; then $exe ; fi
	if [ $SAVE_CODE -eq 0 ]; then rm -rf $my_cpp.cpp ; fi
}
