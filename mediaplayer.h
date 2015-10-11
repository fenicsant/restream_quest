#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QFrame>
#include <QMediaPlayer>

class MediaPlayer : public QFrame
{
  Q_OBJECT
public:
  MediaPlayer(QWidget * parent = 0);
  QSize sizeHint() const;
  ~MediaPlayer();
public slots:
  void playMedia(const QString &path);
  void setVisible(bool visible);
private:
  class Data;
  Data *d;
private slots:
  void error(QMediaPlayer::Error error);
  void durationChanged(qint64 duration);
  void positionChanged(qint64 position);
  void positionMoved(int position);
};

#endif // MEDIAPLAYER_H
