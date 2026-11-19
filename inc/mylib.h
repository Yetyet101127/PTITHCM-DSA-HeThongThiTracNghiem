#ifndef MYLIB_H
#define MYLIB_H
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <cctype>
#include <string.h>
#include <windows.h>

using namespace std;

inline void gotoxy(short x,short y)
{
        HANDLE hConsoleOutput;
        COORD Cursor_an_Pos = { x,y};
        hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleCursorPosition(hConsoleOutput , Cursor_an_Pos);
}

inline void AnHienConTro(bool hien) {
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = hien;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

inline string TrimChuoi(const string &s) {
    size_t a = 0, b = s.length();
    while (a < b && isspace((unsigned char)s[a])) a++;
    while (b > a && isspace((unsigned char)s[b - 1])) b--;
    return s.substr(a, b - a);
}

inline string ChuanHoaKhoangTrang(const string &s) {
    string t = TrimChuoi(s);
    string kq = "";
    bool truocLaCach = false;
    for (size_t i = 0; i < t.length(); i++) {
        if (isspace((unsigned char)t[i])) {
            if (!truocLaCach) kq += ' ';
            truocLaCach = true;
        } else {
            kq += t[i];
            truocLaCach = false;
        }
    }
    return kq;
}

inline string VietHoa(string s) {
    for (size_t i = 0; i < s.length(); i++) s[i] = (char)toupper((unsigned char)s[i]);
    return s;
}

// tach s theo phanCach vao kq[] (toi da toiDa phan tu), tra ve so phan tu; chuoi rong van cho 1 phan tu rong
inline int TachChuoi(const string &s, char phanCach, string kq[], int toiDa) {
    int n = 0;
    string hienTai = "";
    for (size_t i = 0; i < s.length(); i++) {
        if (s[i] == phanCach) {
            if (n < toiDa) kq[n++] = hienTai;
            hienTai = "";
        } else {
            hienTai += s[i];
        }
    }
    if (n < toiDa) kq[n++] = hienTai;
    return n;
}

const int TOI_DA_TRUONG_FILE = 10;

// doc file tung dong (bo dong trong), moi dong goi xuLy(t, n): t[] = cac truong tach bang '|', n = so truong thuc te
// tra ve false neu khong mo duoc file
template<class F> bool DocFileTheoDong(const string &tenFile, F xuLy) {
    ifstream f(tenFile.c_str());
    if (!f.is_open()) return false;
    string dong, t[TOI_DA_TRUONG_FILE];
    while (getline(f, dong)) {
        dong = TrimChuoi(dong);
        if (dong.empty()) continue;
        for (int i = 0; i < TOI_DA_TRUONG_FILE; i++) t[i].clear();
        int n = TachChuoi(dong, '|', t, TOI_DA_TRUONG_FILE);   // n = so truong thuc te
        xuLy(t, n);
    }
    return true;
}

// ---- TEMPLATE DSLK (P = con tro toi nut co truong next) ----
template<class P> int DemNut(P p) {
    int n = 0;
    for (; p != nullptr; p = p->next) n++;
    return n;
}

// nut thu i (tinh tu 0), nullptr neu vuot qua cuoi ds
template<class P> P NutThuI(P p, int i) {
    while (p != nullptr && i-- > 0) p = p->next;
    return p;
}

template<class P> void ThemCuoi(P &dau, P nut) {
    nut->next = nullptr;
    if (dau == nullptr) { dau = nut; return; }
    P q = dau;
    while (q->next != nullptr) q = q->next;
    q->next = nut;
}

// go nut khoi danh sach, khong xoa tren RAM
template<class P> void GoNut(P &dau, P nut) {
    P *p = &dau;
    while (*p != nullptr && *p != nut) p = &(*p)->next;
    if (*p != nullptr) *p = nut->next;
}

template<class P> void GiaiPhongDs(P &dau) {
    while (dau != nullptr) { P t = dau; dau = dau->next; delete t; }
}

template<class P, class KeyFn, class Key>
P TimTrongDs(P dau, KeyFn layKhoa, const Key &khoa) {
    for (P p = dau; p != nullptr; p = p->next)
        if (layKhoa(p) == khoa) return p;
    return nullptr;
}

// SNAPSHOT DSLK
template<class P, class CopyFn> P SaoChepDs(P nguon, CopyFn saoChepNut) {
    P dauMoi = nullptr;
    P *duoi = &dauMoi;
    for (P p = nguon; p != nullptr; p = p->next) {
        *duoi = saoChepNut(p);
        (*duoi)->next = nullptr;
        duoi = &(*duoi)->next;
    }
    return dauMoi;
}

template<class P, class DiffFn> bool DsKhacNhau(P a, P b, DiffFn khacNhau) {
    for (; a != nullptr && b != nullptr; a = a->next, b = b->next)
        if (khacNhau(a, b)) return true;
    return a != nullptr || b != nullptr;
}


// ---- TEMPLATE MANG ----
template<class T> void XoaPhanTuMang(T arr[], int &n, int vt) {
    for (int i = vt; i < n - 1; i++) arr[i] = arr[i + 1];
    arr[--n] = T();
}

template<class T, class KeyFn, class Key>
const T* TimTrongMang(const T snap[], int soSnap, KeyFn layKhoa, const Key &khoa) {
    for (int i = 0; i < soSnap; i++)
        if (layKhoa(snap[i]) == khoa) return &snap[i];
    return nullptr;
}

// SNAPSHOT MANG CON TRO
template<class T, class P, class CopyFn>
void SaoChepMang(T *&snap, int &soSnap, P arr[], int n, CopyFn saoChep) {
    delete[] snap;
    soSnap = n;
    snap = new T[n];
    for (int i = 0; i < n; i++) snap[i] = saoChep(arr[i]);
}

template<class T, class P, class DiffFn>
bool MangKhacNhau(P arr[], int n, const T snap[], int soSnap, DiffFn khacNhau) {
    if (n != soSnap) return true;
    for (int i = 0; i < n; i++)
        if (khacNhau(arr[i], snap[i])) return true;
    return false;
}

#endif