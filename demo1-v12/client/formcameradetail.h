#ifndef FORMCAMERAGRA_H
#define FORMCAMERAGRA_H
#include <QOpenGLWidget>
#include <QWidget>
#include <QJsonObject>
#include "videosource.h"
#include "videowidget.h"
#include "ui_formcameradetail.h"
#include "tool1.h"
#include "alg.h"



#include <QtNetwork>
#include "pd.h"
class ProcessedDataReciver: public QObject{
    Q_OBJECT
public:
    ProcessedDataReciver()
    {
        udp_skt_alg_output=new QUdpSocket(this);
        udp_skt_alg_output->bind(Protocol::SERVER_DATA_OUTPUT_PORT,QUdpSocket::ShareAddress);
        connect(udp_skt_alg_output,SIGNAL(readyRead()),this,SLOT(get_rst()),Qt::QueuedConnection);
    }
signals:
    void send_rst(QByteArray);
public slots:
    void get_rst()
    {
        QByteArray datagram_rst;
        if(udp_skt_alg_output->hasPendingDatagrams())
        {
            //   int size=udp_skt_alg_output->pendingDatagramSize();
            datagram_rst.resize((udp_skt_alg_output->pendingDatagramSize()));
            udp_skt_alg_output->readDatagram(datagram_rst.data(),datagram_rst.size());
            //        udp_skt_alg_output->readDatagram(sss,500);
            //          datagram_rst= udp_skt_alg_output->readAll();
#if 0
            QList <QByteArray > bl= datagram_rst.split(':');
            QByteArray b_index=bl[0];
            int index=*(b_index);

            prt(info,"get cam   %d rst",index);

            QByteArray b_loc=bl[1];
#else

            prt(debug,"get data %s",datagram_rst.data());
            emit send_rst(datagram_rst);
#endif

            //   emit send_camera_rst(index,b_loc);
            //    QList <QByteArray > xy= b_loc.split(',');
            //            int x=xy[0].toInt();
            //            int y=xy[1].toInt();
            //           prt(info," %d : %d",x,y);

        }
    }

private:
    QUdpSocket *udp_skt_alg_output;
    QByteArray rst;
};
class Player:public QThread
{
    Q_OBJECT
public:
    Player(VideoWidget *w)
    {
        wgt=w;
        src=NULL;
        quit=false;
        connect(&rcver,SIGNAL(send_rst(QByteArray)),
                this,SLOT(set_alg_rst(QByteArray)));


    }
    ~Player()
    {
        quit=true;
       this->wait();

    }
    void set_url(QString url)
    {
        if(src==NULL)
            src=new VideoSource(url);
    }

    void set_alg()
    {

    }
public slots:
    void set_alg_rst(QByteArray rst)
    {

        prt(info,"get rst %d bytes:(%c)",rst.length(),rst.data()[0]);
        info.valid=true;
        info.rct=QRect(1,1,wgt->width(),wgt->height());
        info.rst=QRect(1,1,wgt->width()/2,wgt->height()/2);
      }

protected:
    void run()
    {
        src->start();
        Mat rgb_frame;
        Mat bgr_frame;
        //wgt->set data
        while(!quit)
        {
            // src->fetch
            // wgt->update
            QThread::msleep(100);
            //   prt(info,"1");
            // wgt->update1();

            if(src->get_frame(bgr_frame)){
                src->get_frame(bgr_frame);
                cvtColor(bgr_frame,rgb_frame,CV_BGR2RGB);

                QImage  img = QImage((const uchar*)(rgb_frame.data),
                                     rgb_frame.cols,rgb_frame.rows,
                                     rgb_frame.cols*rgb_frame.channels(),
                                     QImage::Format_RGB888);
                wgt->update_data((void*)&info);
                wgt->update_pic(img);
                wgt->update();

            }else{
                //prt(info,"get no frame");
            }

        }
    }
private:
    VideoWidget *wgt;
    bool quit;
    VideoSource *src;
    ProcessedDataReciver rcver;
    layout_info_t info;
    // src
    //url
    //layout data
};
namespace Ui {
class FormCameraDetail;
}
class FormCameraDetail : public QWidget
{
    Q_OBJECT
    typedef struct Camera{
        QString url;
        QJsonObject alg;
    }Camera_t;
    alg_config_t alg_cfg;
    Player *player;
public:
    explicit FormCameraDetail(QWidget *parent = 0);
    ~FormCameraDetail();
    void trans_config()
    {
        alg_cfg.selected_alg=configuration.alg["selected_alg"].toString();
        QString area=configuration.alg["pd"].toObject()["detect_area"].toString();
        QStringList lst=area.split(',');

        if(lst.size()==4){
            alg_cfg.pd.x=lst[0].toInt();
            alg_cfg.pd.y=lst[1].toInt();
            alg_cfg.pd.w=lst[2].toInt();
            alg_cfg.pd.h=lst[3].toInt();
        }
    }

public slots:
    void set_camera(Camera_t cfg)
    {
        player=new Player(ui->openGLWidget);
        prt(info,"get %s",cfg.url.toStdString().data());
        ui->lineEdit_url->setText(cfg.url);
        configuration=cfg;
        trans_config();
        player->set_url(cfg.url);
        player->start();
    }

    void detach_camera()
    {
        if(player){
            delete player;
            player=NULL;
        }
    }
private:

    Ui::FormCameraDetail *ui;
    Camera_t configuration;

};

#endif // FORMCAMERAGRA_H
