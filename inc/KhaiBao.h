#ifndef KHAIBAO_H
#define KHAIBAO_H

#include<iostream>
#include<string>
using namespace std;

const int MAX_LOP=10000;
const string MKSV = "123456";

struct CHT_SV{
	int id;
	char chonluaSV;
};
struct NodeCHT_SV{
	CHT_SV data;
	NodeCHT_SV *next= nullptr;
};
typedef NodeCHT_SV * PTRCTBT; //chi tiet bai thi, moi nut = id cau hoi + dap an SV chon 

struct DIEMTHI{
	string MAMH;
	float Diem;
	PTRCTBT dsCTBT=nullptr;
};

struct NodeDIEMTHI{
	DIEMTHI data;
	NodeDIEMTHI* next=nullptr;
};
typedef NodeDIEMTHI* PTRDIEMTHI;

struct SINHVIEN{
	string HO, TEN, MASV, PHAI, MATKHAU = MKSV;
	PTRDIEMTHI Diem=nullptr;
};
struct NodeSINHVIEN{
	SINHVIEN data;
	NodeSINHVIEN* next=nullptr;
};
typedef NodeSINHVIEN* PTRSINHVIEN;

struct LOP{
	string MALOP, TENLOP;
	PTRSINHVIEN Dssv=nullptr;
};
struct DsLOP{
	int n=0;
	LOP* nodes[MAX_LOP];
	DsLOP(){
		for(int i=0; i<MAX_LOP;i++){
			nodes[i]=nullptr;
		}
	}
};

struct CAUHOI{
	int id;
	string NoiDung, a, b, c, d;
	char DapAn;
	bool DaThi = false;
};
struct Node_CAUHOI{
	CAUHOI data;
	Node_CAUHOI* next=nullptr;
};
typedef Node_CAUHOI* PTRCAUHOI;

struct MONHOC{
	string MAMH, TENMH;
	PTRCAUHOI dsCHT=nullptr;
};
struct Node_MONHOC{
	MONHOC data;
	Node_MONHOC* right=nullptr;
	Node_MONHOC* left=nullptr;
};
typedef Node_MONHOC* treeMH;


// DSLK
template<class P> int DemNut(P p) {
    int n = 0;
    for (; p != nullptr; p = p->next) n++;
    return n;
}

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


// MANG CON TRO
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