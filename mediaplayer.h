#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QFrame>
#include <QMediaPlayer>

//! Скомпонованная панель вывода видео-информации.
/*! Класс представляет собой медиа проигрыватель объединенный с элементами
 * управления. */
class MediaPlayer : public QFrame
{
  Q_OBJECT
public:
    //! Конструктор.
  MediaPlayer(QWidget * parent = 0);
    //! Размер требуемый для отображения видео.
  QSize sizeHint() const;
    //! Деструктор.
  ~MediaPlayer();
public slots:
    //! Открывает файл для вывода на экран.
  void playMedia(const QString &path);
    //! Останавливает вывод при скрытии.
  void setVisible(bool visible);
private:
    //! Класс закрытых данных.
  class Data;
    //! Закрытые данные класса.
  Data *d;
private slots:
    //! Вызывается при ошибках QMediaPlayer.
  void error(QMediaPlayer::Error error);
    //! Вызывается при изменении продолжительности фильма (условно при загрузке фильма).
  void durationChanged(qint64 duration);
    //! Вызывается при изменении текущей позиции проигрывании.
  void positionChanged(qint64 position);
    //! Вызывается при выборе текущей позиции фильма пользователем.
  void positionMoved(int position);
};

//! \file mediaplayer.h Содержит простой видео-проигрыватель.

#endif // MEDIAPLAYER_H
