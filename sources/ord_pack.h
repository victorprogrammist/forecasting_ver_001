
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#ifndef ORD_PACK_H
#define ORD_PACK_H

#include <QtCore>
#include "dispersion.h"

template <class N>
struct OrdPack {

    int cnt_values = 0;
    int c_invalid = 0;

    OrdPack() {}

    OrdPack(int ma_c) : ma_count_for_pack(ma_c), has_koef_bound(false) {}

    OrdPack(int ma_c, double k_lo_bound, double hi_bnd)
        : ma_count_for_pack(ma_c)
        , koef_lo_bound(k_lo_bound)
        , hi_bound(hi_bnd)
        , has_koef_bound(true) { myassert( koef_lo_bound >= 1.0 ); }

    std::map<double,int> map_counter;
    std::multimap<double,N> mmap_ord;
    double mi_koef = 0;
    int c_numbers = 0;

    void add_value(double koef, const N& data);

    // выдает значения в порядке уменьшения koef
    QVector<N> get_values() const;

private:
    double lo_bound() const {
        if (has_koef_bound==false) return mi_koef;
        return hi_bound - (hi_bound - mi_koef)*koef_lo_bound; }

    bool has_koef_bound = false;
    int ma_count_for_pack = 0;
    double koef_lo_bound = 0;
    double hi_bound = 0;
};

template <class N>
void OrdPack<N>::add_value(double koef, const N& data) {
    if (ma_count_for_pack == 0) return;

    myassert( ma_count_for_pack > 0 );

    ++cnt_values;

    if (std::isfinite(koef)==false) {
        ++c_invalid;
        return;
    }

    if (c_numbers < ma_count_for_pack) {
        mmap_ord.insert({-koef,data});

        if (has_koef_bound && koef > hi_bound)
            return;

        if (c_numbers == 0)
            mi_koef = koef;
        else
            mi_koef = std::min(mi_koef, koef);

        ++c_numbers;
        ++map_counter[koef];

        return;
    }

    if (koef >= lo_bound())
        mmap_ord.insert({-koef,data});

    if (koef < mi_koef)
        return;

    if (has_koef_bound && koef > hi_bound)
        return;

    ++map_counter[koef];
    ++c_numbers;

    int c_after_erase = c_numbers - map_counter.begin()->second;
    if (c_after_erase < ma_count_for_pack) return;

    map_counter.erase(map_counter.begin());
    c_numbers = c_after_erase;

    mi_koef = map_counter.begin()->first;

    double lb = lo_bound();
    while (mmap_ord.empty()==false) {
        double mk = -(--mmap_ord.end())->first;
        if ( mk >= lb ) break;
        mmap_ord.erase(--mmap_ord.end());
    }
}

template <class N>
QVector<N> OrdPack<N>::get_values() const {

    QVector<N> res(mmap_ord.size());
    int c = 0;

    for (auto p : mmap_ord) res[c++] = p.second;

    return res;
}

#endif // ORD_PACK_H
