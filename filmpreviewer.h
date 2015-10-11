#ifndef FILMPREVIEWER_H
#define FILMPREVIEWER_H

#include <QWidget>
class FilmListItem;
class QMediaPlayer;
class QMediaPlaylist;
namespace Ui {
class FilmPreviewer;
}

class FilmPreviewer : public QWidget
{
  Q_OBJECT
public:
  FilmListItem *film() const {return ffilm;}
  void setFilm(FilmListItem *value);

  explicit FilmPreviewer(QWidget *parent = 0);
  ~FilmPreviewer();
signals:

public slots:
private:
  Ui::FilmPreviewer *ui;
  FilmListItem *ffilm;
  QMediaPlayer *player;
  QMediaPlaylist *playlist;
};

#endif // FILMPREVIEWER_H
