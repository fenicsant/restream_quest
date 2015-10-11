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
signals:
    //! Сигнал окончания загрузки.
  void reloadFinished();
public slots:
    //! Инициирует обновление списка фильмов.
  void beginReload();
    //! Закрывает окно более детального просмотра информации о фильме.
  void closePreview();
protected:
  void resizeEvent(QResizeEvent *);
private:
  class Data;
  Data *d;
  enum {
    PreviewIdentSize = 50
  };
private slots:
  void pageReady(int page, const QByteArray & content);
  void pageError(int page, const QString & errorStr);
  void fileFinished(const InetFileTaskId & id);
  void fileError(const InetFileTaskId & id, const QString & errorStr);
};

#endif // FILMLISTVIEW_H
