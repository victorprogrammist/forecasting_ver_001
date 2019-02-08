
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#ifndef TLS_H
#define TLS_H

#include <QtCore>

using std::pair;

using ArNums = QVector<double>;
using ArInts = QVector<int>;

inline double epsilon() { return 1e-10; }

#define myassert(expr) \
     (static_cast<bool>(expr)?void(0):do_throw_2(#expr, __FILE__, __LINE__, __func__))

[[noreturn]] inline void do_throw_2(const QString& msg, const char* file, int line, const char* func) {
    QString s = msg.trimmed() + ", "+ file + ", " + QString::number(line) + ", " + func;
    throw std::runtime_error(s.toStdString());
}

[[noreturn]] inline void do_throw(const QString& msg) {
    throw std::runtime_error(msg.toStdString());
}

inline QString procent(double v) {
    return QString::number(v*100, 'f', 5) + "%";
}

inline double pow2(double v) { return v*v; }

template <class A> double dbl(A v) { return static_cast<double>(v); }

template <class A, class B> double safe_div(A v1, B v2) {

    if (v2 == 0)
        do_throw("Деление на ноль");

    return dbl(v1) / dbl(v2);
}

ArInts string_to_ar_ints(const QString& s);

QString get_file_text(const QString& file_name);
QStringList get_file_lines(const QString& file_name);
QStringList get_file_col(const QString& file_name, const QString& col, const QString& spl);
ArNums get_file_col_num(const QString& file_name, const QString& col, const QString& spl);

double sqrt_sign(double v);

#endif // TLS_H
