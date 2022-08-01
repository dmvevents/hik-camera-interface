#ifndef HKHELPER_H
#define HKHELPER_H

//#include <QObject>
//#include <QDebug>
//#include <QMutex>
//#include <QTimerEvent>
#include "HCNetSDK.h"
//#include "plaympeg4.h"
#include <opencv2/opencv.hpp>
#include "HCNetSDK.h"
#include "LinuxPlayM4.h"
class HKHelper
{
public:

    explicit HKHelper();

    void real_play_by_win(char *ip, long port, char *user_name, char *password,HWND win_id);
    void stop_play(LONG lRealPlayHandle);
    void device_logout(LONG lUserID);
    void play_custom(const char *ip, long port, const char *user_name , const char *password);




private:
    void uninit_SDK();
    LONG device_login(const char *ip, long port, const char *user_name , const char *password);
    void show_custom(LONG lUserID);


private:
    bool is_inited;


public:
    static HKHelper * s_this;
    static LONG g_nPort;

    static void HCSdkDecCallBackMend(int nPort, char* pBuf, int nSize, FRAME_INFO* pFrameInfo, void * nUser, int nReserved2);
    static void HCSdkRealImage(LONG,DWORD,BYTE *,DWORD,void *);
    void init();
//    void timerEvent(QTimerEvent *);
    void decode_image_cv(cv::Mat img);


};

#endif // HKHELPER_H
