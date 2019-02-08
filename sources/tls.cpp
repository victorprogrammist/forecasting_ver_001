
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#include "tls.h"

ArInts string_to_ar_ints(const QString& s) {
    ArInts res;
    for (auto ss : s.split(QRegExp("[;,]")))
        if (ss.isEmpty()==false)
            res.append(ss.toInt());
    res.squeeze();
    return res;
}

double sqrt_sign(double v) {
    if (v >= 0) return sqrt(v);
    return -sqrt(-v);
}

QByteArray file_read_raw(const QString& fn) {
    QFile file(fn);

    if (file.exists() == false)
        do_throw("file not found: "+fn);

    if (file.open(QIODevice::ReadOnly) == false)
        do_throw("file not openned: "+fn);

    return file.read(file.size());
}

QString get_file_text(const QString& file_name) {
    auto ba = file_read_raw(file_name);
    return QTextCodec::codecForName("cp1251")->toUnicode(ba);
}

QStringList get_file_lines(const QString& file_name) {
    return get_file_text(file_name).split(QRegExp("\r?\n"), QString::SkipEmptyParts);
}

QStringList get_file_col(const QString& file_name, const QString& col, const QString& spl) {

    QStringList list = get_file_lines(file_name);

    QStringList res;

    if (list.isEmpty()) return res;

    int i_col = list.at(0).split(spl).indexOf(col);
    if (i_col < 0) return res;

    for (int i = 1; i < list.size()-1; ++i)
        res.append(list.at(i).split(spl).at(i_col));

    return res;
}

ArNums get_file_col_num(const QString& file_name, const QString& col, const QString& spl) {

    QStringList ss = get_file_col(file_name, col, spl);
    ArNums res(ss.size());

    for (int i = 0; i < ss.size(); ++i)
        res[i] = ss.at(i).toDouble();

    return res;
}
