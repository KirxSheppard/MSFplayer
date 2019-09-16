#include "videowidget.h"
#include <QDebug>
#include <QString>

VideoWidget::VideoWidget(QWidget *parent)
{
    setMouseTracking(true);
}

void VideoWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    const QSize imgS = m_imgGOps.size();
    QSize s = imgS.scaled(size(), Qt::KeepAspectRatio);

    //background
//    p.fillRect(this->rect(), mBgColor);

    p.drawImage(QRect(QPoint((width() - s.width()) / 2, (height() - s.height()) / 2), s), m_imgGOps);
}

void VideoWidget::setImage(const QImage &img)
{
    //only two items (current and prev)
    if (m_imgs.size() >= 2)
        m_imgs.removeFirst();
    else if (m_imgs.empty()) //allows to display photos
        m_imgs.push_back(img);
    m_imgs.push_back(img);

    Q_ASSERT(m_imgs.size() == 2);

    if (m_imgGOps.size() != m_imgs[1].size() || m_imgGOps.format() != m_imgs[1].format())
        m_imgGOps = QImage(m_imgs[1].size(), m_imgs[1].format());

    updateVidEffects();
}

void VideoWidget::updateVidEffects()
{
    QPainter painter(&m_imgGOps);

    const QSize imgS = m_imgGOps.size();

    painter.drawImage(QRect(QPoint(), imgS), m_imgs[1]);

    if (ifOnionSkinning)
    {
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setOpacity(mOsOpacityVal);
        painter.drawImage(QRect(QPoint(), imgS), m_imgs[0]);
    }
    if(ifWaterMark)
    {
        QImage waterMarkImg(mWaterMarkPath);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setOpacity(mWmOpacityVal);
        if(waterMarkImg.height() > imgS.height())
        {
            painter.end();
            emit clearWmPath();
            updateVidEffects();
        }
        else
        {
            //here i can control watermark position and scale
            painter.drawImage(imgS.width()  / 2 - waterMarkImg.width()  * ((double)mWmScale / 100) / 2 + mWmPosX,
                              imgS.height() / 2 - waterMarkImg.height() * ((double)mWmScale / 100) / 2 + mWmPosY,
                              waterMarkImg.scaled(waterMarkImg.width()  * ((double)mWmScale / 100),
                                                  waterMarkImg.height() * ((double)mWmScale / 100),
                                                  Qt::KeepAspectRatio));
            painter.end();
        }
    }
    emit afterUpdateIfPaused();
    emit checkIfSaving();
    update();

}

void VideoWidget::onionSkinning()
{
    ifOnionSkinning = !ifOnionSkinning;
    updateVidEffects();
}

void VideoWidget::waterMark()
{
    ifWaterMark = !ifWaterMark;
    updateVidEffects();
    update();
}

void VideoWidget::msfWaterMarkPath(QString path)
{
    mMsfLogoPath = path;
    mWaterMarkPath = mMsfLogoPath;
}

void VideoWidget::saveFrames(QString temp, int frameSaveCounter, int pos)
{
    m_imgGOps.save(QString(temp.insert(pos,"_%1")).arg(frameSaveCounter, 6, 10, QLatin1Char('0')));
}

void VideoWidget::setWmPosX(int value)
{
    mWmPosX = value;
    updateVidEffects();
}

void VideoWidget::setWmPosY(int value)
{
    mWmPosY = value;
    updateVidEffects();
}

void VideoWidget::resetWmPath(bool value)
{
    mWaterMarkPath = mMsfLogoPath;
    updateVidEffects();
}

void VideoWidget::setWmPath(QString path)
{
    mWaterMarkPath = path;
    updateVidEffects();
}

void VideoWidget::setWmScaleValue(int value)
{
    mWmScale = value;
    updateVidEffects();
}

void VideoWidget::setOsOpacityValue(double value)
{
    mOsOpacityVal = value;
    updateVidEffects();
}

void VideoWidget::setWmOpacityValue(double value)
{
    mWmOpacityVal = value;
    updateVidEffects();
}
