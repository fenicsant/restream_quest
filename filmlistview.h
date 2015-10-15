#ifndef FILMLISTVIEW_H
#define FILMLISTVIEW_H


class QByteArray;
class QString;

class InetFile;
struct InetFileTaskId;
class FilmListItem;
class FilmPreviewer;

#include <QWidget>

//! Класс предназначен для загрузки, обновления и отображения списка фильмов.
class FilmListView : public QWidget
{
  Q_OBJECT
public:
    //! Вызывает окно более детального простмотра информации о фильме.
  void previewFilm(FilmListItem *film);
    //! Конструктор.
  explicit FilmListView(QWidget *parent = 0);
  ~FilmListView();
signals:
    //! Сигнал окончания загрузки.
  void reloadFinished();
public slots:
    //! Инициирует обновление списка фильмов.
  void beginReload();
    //! Закрывает окно более детального просмотра информации о фильме.
  void closePreview();
protected:
    //! Вызывается при изменении размеров окна. Необходим для корректного отображения превью.
  void resizeEvent(QResizeEvent *);
private:
    //! Класс закрытых данных.
  class Data;
    //! Закрытые данные класса.
  Data *d;
  enum {
    //! Смещение окна детальной информации от края окна.
    PreviewIdentSize = 50
  };
  Q_DISABLE_COPY(FilmListView)
private slots:
    //! Вызывается при получении данных страницы фильмов.
  void pageReady(int page, const QByteArray & content);
    //! Вызывается при ошибке получения данных страницы фильмов.
  void pageError(int page, const QString & errorStr);
    //! Вызывается при получении другой информации.
  void fileFinished(const InetFileTaskId & id);
    //! Вызывается при ошибках получения информации.
  void fileError(const InetFileTaskId & id, const QString & errorStr);
};

//! \file filmlistview.h Основной компонент отображения списка фильмов.

#endif // FILMLISTVIEW_H
