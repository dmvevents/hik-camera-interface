#include "hkhelper.h"


std::mutex Mutex;
cv::Mat g_BGRImage;

void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
    char tempbuf[256] = {0};
//    Q_UNUSED(tempbuf);
    switch(dwType)
    {
    case EXCEPTION_RECONNECT:    //预览时重连
        std::cout <<std::string("----------reconnect--------%1\n");
        break;
    default:
        break;
    }
}
HKHelper* HKHelper::s_this = nullptr;
HKHelper::HKHelper()
{
    //初始化海康SDK
    is_inited = NET_DVR_Init();
    std::cout  << "SDK init ok" << std::endl;
    //设置连接时间与重连时间
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(10000, true);
    //设置异常消息回调函数
    NET_DVR_SetExceptionCallBack_V30(0, NULL,g_ExceptionCallBack, NULL);
    s_this = this;
}





void HKHelper::real_play_by_win(char *ip, long port, char *user_name, char *password,HWND win_id)
{

    LONG lUserID = device_login( ip,  port,  user_name,  password);

    if (lUserID < 0)
    {
        std::cout <<"Login failed, error code: " << NET_DVR_GetLastError() << std::endl;
        NET_DVR_Cleanup();
        return;
    }

    //---------------------------------------
    //启动预览并设置回调数据流
    LONG lRealPlayHandle;
    NET_DVR_PREVIEWINFO struPlayInfo = {0};
    struPlayInfo.hPlayWnd = win_id;       //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
    struPlayInfo.lChannel     = 1;       //预览通道号
    struPlayInfo.dwStreamType = 0;       //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
    struPlayInfo.dwLinkMode   = 0;       //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
    struPlayInfo.bBlocked     = 1;       //0- 非阻塞取流，1- 阻塞取流

    //窗体句柄解码
    lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, NULL, NULL);
    if (lRealPlayHandle < 0)
    {
        std::cout <<"NET_DVR_RealPlay_V40 error\n" << std::endl;
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return;
    }
    //---------------------------------------
    return;
}


LONG HKHelper::device_login(const char *ip, long port, const char *user_name,const char *password)
{
    //登录参数，包括设备地址、登录用户、密码等
    NET_DVR_USER_LOGIN_INFO struLoginInfo;
    struLoginInfo.bUseAsynLogin = 0; //同步登录方式
    strcpy(struLoginInfo.sDeviceAddress, ip); //设备IP地址
    struLoginInfo.wPort = port; //设备服务端口
    strcpy(struLoginInfo.sUserName, user_name); //设备登录用户名
    strcpy(struLoginInfo.sPassword, password); //设备登录密码

    //设备信息, 输出参数
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40;

    LONG lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
    return lUserID;
}

void HKHelper::stop_play(LONG lRealPlayHandle)
{
    //关闭预览
    NET_DVR_StopRealPlay(lRealPlayHandle);
}

void HKHelper::uninit_SDK()
{
    //释放SDK资源
    NET_DVR_Cleanup();
}

void HKHelper::device_logout(LONG lUserID)
{
    //注销用户
    NET_DVR_Logout(lUserID);
}


LONG HKHelper::g_nPort=-100;
void HKHelper::HCSdkDecCallBackMend(int nPort, char* pBuf, int nSize, FRAME_INFO* pFrameInfo, void *nUser, int nReserved2)
{
    if (pFrameInfo->nType == T_YV12)
    {
        std::cout  << "the frame infomation is T_YV12" << std::endl;
        int width = pFrameInfo->nWidth;
        int height = pFrameInfo->nHeight;
        cv::Mat sMat(height+height/2,width,CV_8UC1,(char *)pBuf);
        cvtColor(sMat,sMat,cv::COLOR_YUV2RGB_YV12);
        if(!sMat.empty())
        {
            Mutex.lock();
            g_BGRImage.release();
            g_BGRImage = sMat;
            Mutex.unlock();
        }
    }
}




void HKHelper::play_custom(const char *ip, long port, const char *user_name , const char *password)
{

    //数据解码回调函数，
    //功能：将YV_12格式的视频数据流转码为可供opencv处理的BGR类型的图片数据，并实时显示

    //登录函数，用作摄像头id以及密码输入登录
    LONG lUserID = device_login(ip, port, user_name, password);
    //bool HK_camera::Login(const char* sDeviceAddress,const char* sUserName,const char* sPassword, WORD wPort);        //登陆（VS2017版本）


    if (lUserID>=0)//用户名以及密码，根据此系列博客一中的方法查看或设置
    {
        std::cout  << "login successfully" << std::endl;
        this->show_custom(lUserID);
    }
    else
    {
        std::cout  << "login fail" << std::endl;
    }

}

void HKHelper::show_custom(LONG lUserID)
{

    if (PlayM4_GetPort(&g_nPort))            //获取播放库通道号
    {
        if (PlayM4_SetStreamOpenMode(g_nPort, STREAME_REALTIME))      //设置流模式
        {
            if (PlayM4_OpenStream(g_nPort, NULL, 0, 1024 * 1024))         //打开流
            {

                if (PlayM4_SetDecCallBackExMend(g_nPort,HCSdkDecCallBackMend, NULL, 0, NULL))

                {
                    if (PlayM4_Play(g_nPort, NULL))
                    {
                        std::cout << "success to set play mode" << std::endl;
                    }
                    else
                    {
                        std::cout << "fail to set play mode" << std::endl;
                    }
                }
                else
                {
                    std::cout << "fail to set dec callback " << std::endl;
                }
            }
            else
            {
                std::cout << "fail to open stream" << std::endl;
            }
        }
        else
        {
            std::cout << "fail to set stream open mode" << std::endl;
        }
    }
    else
    {
        std::cout << "fail to get port" << std::endl;
    }
    //启动预览并设置回调数据流
    NET_DVR_PREVIEWINFO struPlayInfo = { 0 };
    struPlayInfo.hPlayWnd = NULL; //窗口为空，设备SDK不解码只取流
    struPlayInfo.lChannel = 1; //Channel number 设备通道
    struPlayInfo.dwStreamType = 0;// 码流类型，0-主码流，1-子码流，2-码流3，3-码流4, 4-码流5,5-码流6,7-码流7,8-码流8,9-码流9,10-码流10
    struPlayInfo.dwLinkMode = 0;// 0：TCP方式,1：UDP方式,2：多播方式,3 - RTP方式，4-RTP/RTSP,5-RSTP/HTTP
    struPlayInfo.bBlocked = 1; //0-非阻塞取流, 1-阻塞取流, 如果阻塞SDK内部connect失败将会有5s的超时才能够返回,不适合于轮询取流操作.

    if (NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, HCSdkRealImage, NULL))
    {
        //        cv::namedWindow("RGBImage2");
    }
}

//实时视频码流数据获取 回调函数
void HKHelper::HCSdkRealImage(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void* pUser)
{
    if (dwDataType == NET_DVR_STREAMDATA)//码流数据
    {
        if (dwBufSize > 0 && HKHelper::g_nPort != -1)
        {
            if (!PlayM4_InputData(HKHelper::g_nPort, pBuffer, dwBufSize))
            {
                std::cout << "fail input data" << std::endl;
            }
            else
            {
                std::cout << "success input data" << std::endl;
            }

        }
    }
}

//void HKHelper::timerEvent(QTimerEvent *)
//{
//    Mutex.lock();
//    if(!g_BGRImage.empty())
//    {
//        cv::imshow("123",g_BGRImage);
//    }

//    Mutex.unlock();
//}

