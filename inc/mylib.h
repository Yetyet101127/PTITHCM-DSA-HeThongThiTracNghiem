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

inline void clrscr() {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = {0, 0};
    DWORD count;
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
        DWORD cellCount = csbi.dwSize.X * csbi.dwSize.Y;
        FillConsoleOutputCharacter(hStdOut, (TCHAR) ' ', cellCount, coord, &count);
        FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, cellCount, coord, &count);
        SetConsoleCursorPosition(hStdOut, coord);
    }
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

template<class F> bool DocFileTheoDong(const string &tenFile, F xuLy) {
    ifstream f(tenFile.c_str());
    if (!f.is_open()) return false;
    string dong, t[TOI_DA_TRUONG_FILE];
    while (getline(f, dong)) {
        dong = TrimChuoi(dong);
        if (dong.empty()) continue;
        for (int i = 0; i < TOI_DA_TRUONG_FILE; i++) t[i].clear();   // truong thieu = "", khong giu du lieu dong truoc
        int n = TachChuoi(dong, '|', t, TOI_DA_TRUONG_FILE);   // n = so truong that su co tren dong
        xuLy(t, n);
    }
    return true;
}

#endif