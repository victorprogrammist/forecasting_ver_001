
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#include "file_loader.h"

QVector<QString> FileLoader::get_cat_cols(const QString& cols, const QString& separator) {

    QStringList ss = cols.split(col_splitter);
    if (ss.size() == 1) return get_col(cols);

    QVector<QString> res(count_loaded_data);

    QVector<QVector<QString>*> cols_data;

    for (QString s : ss) cols_data.append(&get_col(s));

    for (int i_data = 0; i_data < count_loaded_data; ++i_data) {
        QStringList lst;
        for (auto cl : cols_data) lst.append(cl->at(i_data));
        res[i_data] = lst.join(separator);
    }

    return res;
}

QVector<QString>& FileLoader::get_col(const QString& col) {
    auto itr = map_loaded_data.find(col);
    if (itr == map_loaded_data.end()) do_throw("Колонка отсутствует: "+col);
    return itr.value();
}

ArNums FileLoader::get_col_nums(const QString& col) {
    QVector<QString>& ss = get_col(col);
    ArNums res(ss.size());
    for (int i = 0; i < ss.size(); ++i)
        res[i] = ss.at(i).toDouble();
    return res;
}

void FileLoader::private_append_one_file(const QString& file_name) {

    QStringList list = get_file_lines(file_name);
    if (list.isEmpty()) return;

    QStringList cc = list.at(0).split(col_splitter);

    if (list_cols.isEmpty())
        list_cols = cc;
    else if (list_cols.join(",") != cc.join(","))
        do_throw("колонки в загружаемых файлах не совпадают");

    list.removeFirst();

    int new_count_data = count_loaded_data + list.size();

    QVector<QVector<QString>*> ref_data;
    for (int i = 0; i < list_cols.size(); ++i) {
        QVector<QString>& lst = map_loaded_data[list_cols.at(i)];
        lst.resize(new_count_data);
        ref_data.append(&lst);
    }

    for (int i_line = 0; i_line < list.size(); i_line++) {

        QString s = list.at(i_line);
        if (s.isEmpty()) continue;

        QStringList ss = s.split(col_splitter);
        if (ss.size() != list_cols.size())
            do_throw("Ошибка в файле "+file_name+" в строке "+QString::number(i_line));

        for (int i_col = 0; i_col < ref_data.size(); ++i_col)
            ref_data[i_col]->data()[count_loaded_data+i_line] = ss.at(i_col).trimmed();
    }

    count_loaded_data = new_count_data;
}

void FileLoader::load_files(const QString& dir_with_data_or_file) {

    if (dir_with_data_or_file.isEmpty()) do_throw("Указно пустое имя файла");

    if (QDir(dir_with_data_or_file).exists()==false) {
        if (QFile::exists(dir_with_data_or_file)) {
            private_append_one_file(dir_with_data_or_file);
            return;
        }}

    QString s = dir_with_data_or_file;
    if (s.back() != '/' && s.back() != '\\') s += QDir::separator();

    QStringList lst_fn = QDir(s).entryList(QDir::Files);
    lst_fn.sort(Qt::CaseInsensitive);
    for (QString fn: lst_fn)
        private_append_one_file(s + fn);
}

