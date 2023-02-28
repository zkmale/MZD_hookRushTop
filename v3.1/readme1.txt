
因使用海康SDK调用视频流太消耗资源，所以使用opencv调用视频流，SDK控制摄像头。
原始球机控制参数：
   "Pan": 55,
   "Tilt": 47,
   "Zoom": 1
原始冲顶阈值参数：
    "serious_punch_top": 2000,
    "moderate_punch_top": 1600,
    "mild_punch_top": 1200
	
还添加了是否控制球机追踪吊钩的功能。如果is_contral_cam == 1的时候就可以追踪吊钩，当contral_cam_threadshold达到某个角度时，回归初始化。

export LD_LIBRARY_PATH=/home/mzd/project/ken/vulkan/1.2.162.1/x86_64/lib:./mylib:/home/mzd/project/ken/hikvisionSDK/CH-HCNetSDKV6.1.9.4_build20220413_linux64/lib:/home/mzd/project/ken/json/build/lib:$LD_LIBRARY_PATH

/usr/bin/g++ -o hook_rush_top_json hook_rush_top.cpp base64.cpp RrConfig.cpp WindNetPredictDetect.cpp cJSON.c -I ./ -I ./myinclude -I ./base64 -I /home/mzd/project/ken/json/include/json -I /home/mzd/project/ken/ncnn_build_2004_vulkan/include -I /home/mzd/project/ken/ncnn_build_2004_vulkan/include/ncnn -I /home/mzd/project/ken/ncnn_build_2004_vulkan/include/glslang -I /home/mzd/project/ken/ncnn_build_2004_vulkan/include/glslang/Include -I /home/mzd/project/ken/opencv_build_2004/include -I /home/mzd/project/ken/opencv_build_2004/include/opencv -I /home/mzd/project/ken/opencv_build_2004/include/opencv2 -I /home/mzd/project/ken/vulkan/1.2.162.1/x86_64/include -I /home/mzd/project/ken/vulkan/1.2.162.1/x86_64/include/vulkan -I /home/mzd/project/ken/vulkan/1.2.162.1/x86_64/include/glslang -I /home/mzd/project/ken/hikvisionSDK/CH-HCNetSDKV6.1.9.4_build20220413_linux64/include -I /home/mzd/project/ken/hikvisionSDK/CH-HCNetSDKV6.1.9.4_build20220413_linux64/demo/1-C++developdemo/QtDemo/includeCn -L /home/mzd/project/ken/opencv_build_2004/lib -L /home/mzd/project/ken/ncnn_build_2004_vulkan/lib -L /home/mzd/project/ken/vulkan/1.2.162.1/x86_64/lib -L /home/mzd/project/ken/hikvisionSDK/CH-HCNetSDKV6.1.9.4_build20220413_linux64/lib -L /home/mzd/project/ken/hikvisionSDK/CH-HCNetSDKV6.1.9.4_build20220413_linux64/lib/HCNetSDKCom -L /home/mzd/project/ken/json/build/lib -ljsoncpp -lncnn -lglslang -lvulkan -lSPIRV -lGenericCodeGen -lMachineIndependent -lOGLCompiler -lopencv_imgcodecs -lopencv_video -lopencv_imgproc -lopencv_core -lopencv_highgui -lopencv_videoio -lgomp -lpthread -std=c++11 -lshaderc_shared -lAudioRender -lhpr -lNPQos -lSuperRender -lz -lHCCore -lhcnetsdk -lPlayCtrl -lHCGeneralCfgMgr -lHCPreview -lX11


