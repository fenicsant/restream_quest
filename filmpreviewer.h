#ifndef FILMPREVIEWER_H
#define FILMPREVIEWER_H

#include <QWidget>
class FilmListItem;
class QMediaPlayer;
class QMediaPlaylist;
namespace Ui {
class FilmPreviewer;
}

//! Отображает более детальную информации о фильме.
class FilmPreviewer : public QWidget
{
  Q_OBJECT
public:
    //! Отображаемый фильм.
  FilmListItem *film() const {return ffilm;}
    //! Устанавливает фильм для отображения
  void setFilm(FilmListItem *value);
    //! Конструктор.
  explicit FilmPreviewer(QWidget *parent = 0);
    //! Деструктор.
  ~FilmPreviewer();
private:
    //! Формочка из ui-файла.
  Ui::FilmPreviewer *ui;
    //! Поле хранения отображаемого фильма.
  FilmListItem *ffilm;
  Q_DISABLE_COPY(FilmPreviewer)
};

//! \file filmpreviewer.h Выводит более детальную информацию о фильме.

#endif // FILMPREVIEWER_H
