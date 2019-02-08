
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#ifndef FILE_LOADER_H
#define FILE_LOADER_H

#include <QtCore>
#include "tls.h"

struct FileLoader {

    QRegExp col_splitter = QRegExp("[,;]");

    QStringList list_cols;

    int count_loaded_data = 0;

    // если указан каталог, то будут загружены
    //   все файлы из указанного каталога в порядке
    //   сортировки названий файлов.
    //   Предполагается, что все файлы к загрузке и имеют одинаковые колонки.
    // допускаются повторные вызовы с указанием новых файлов к добавлению
    void load_files(const QString& dir_with_data_or_file);

    QVector<QString>& get_col(const QString& col);
    ArNums get_col_nums(const QString& col);
    QVector<QString> get_cat_cols(const QString& cols, const QString& separator);

private:
    QMap<QString,QVector<QString>> map_loaded_data;
    void private_append_one_file(const QString& file_name);
};

#endif // FILE_LOADER_H
