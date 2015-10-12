#ifndef FILMLISTITEM_H
#define FILMLISTITEM_H

class QWidget;
class QLagel;
class FilmListView;

#include <QString>
#include <QMap>
#include <QVariant>

//! Хранилище информации по фильму
/*! Класс производит загрузку информации, хранение, и формирование отображения
 * ее в списке фильмов */
class FilmListItem
{
public:
    //! Создает и хранит виджет и выводит на него информаци о фильме.
  QWidget * getAsWidget();
  enum {
    PosterFixetWidth = 150  //!< Ширина обложки при отображении.
  };
    // Загружаемые свойства
  bool adult;           //!< Недецкое.
  int id;               //!< Идентификатор фильма.
  QStringList genres;   //!< Присутсвующие жанры.
  QString title;        //!< Название фильма.
  QString overview;     //!< Краткое содержание фильма.
  QString poster_path;  //!< Адрес обложки фильма.
  QString video;        //!< Адрес превью фильма.

                        //! Другие характеристики фильма (имя - значение).
  QMap<QString, QVariant> property;
    //! Возвращает загруженную обложку фильма
  const QPixmap & poster() const;
    //! Устанавливает обложку фильма.
  void setPoster(const QPixmap &value);
    //! Устанавливает обложку фильма.
  void setPoster(const QByteArray &value);
    //! Производит загрузку данных из объекта json.
  bool load(const QJsonObject &json);
    //! Разбирает список жанров фильма.
  static QStringList loadGenre(const QJsonArray &json);
    //! Перечисление строковых констант из jsonNames().
  enum {
    JsonNId,
    JsonNAdult,
    JsonNGenres,
    JsonNGenreIds,
    JsonNTitle,
    JsonNOverview,
    JsonNPoster,
    JsonNVideo
  };
    //! Список строковых констант для разбора структуры json.
  static const QVector<QString> &jsonNames();
    //! Возвращает строку - "постер не найден".
  static const QString &noposter();
    //! Конструктор
  FilmListItem(FilmListView *parent);
private:
    //! Структура закрытых данных.
  class Data;
    //! Закрытые данные класса.
  Data *d;
};

//! \file filmlistitem.h Описывает элемент списка фильмов.

#endif // FILMLISTITEM_H
