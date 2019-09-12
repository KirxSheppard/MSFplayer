#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QPainter>

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    VideoWidget(QWidget *parent = nullptr);
//    VideoWidget(const QString msfLogo, QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override;
    void updateVidEffects();
    void mPlayVidToUpdate();
    void mPauseVidAfterUpdate();
    void onionSkinning();
    void waterMark();
    void msfWaterMarkPath(QString path);
    void saveFrames(QString temp, int frameSaveCounter, int pos);

    //Slots
    void setWmPosX(int value);
    void setWmPosY(int value);
    void resetWmPath(bool value);
    void setWmPath(QString path);
    void setImage(const QImage &img);
    void setWmOpacityValue(double value);
    void setWmScaleValue(int value);


public slots:

signals:
    void pauseVidAfterUpdate(bool val);
    void clearWmPath();
    void afterUpdateIfPaused();
    void checkIfSaving();


private:
    QList<QImage> m_imgs;
    QImage m_imgGOps;
    QColor mBgColor = Qt::black;
    QString mWaterMarkPath, mMsfLogoPath;
    bool ifPaused, ifWaterMark, ifOnionSkinning;
    int mWmPosX = 0, mWmPosY = 0;
    int mWmScale = 100;
    double mWmOpacityVal = 0.3;

};

#endif // VIDEOWIDGET_H
