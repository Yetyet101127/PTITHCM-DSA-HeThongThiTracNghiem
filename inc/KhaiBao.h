#ifndef KHAIBAO_H
#define KHAIBAO_H

#include<iostream>
#include<string>
using namespace std;

const int MAX_LOP=10000;
const string MKSV = "123456";


struct CHT_SV{
	int id;
    int tronDapAn[4] = {1, 2, 3, 4};
	char chonluaSV;
};
struct NodeCHT_SV{
	CHT_SV data;
	NodeCHT_SV *next= nullptr;
};
typedef NodeCHT_SV * PTRCTBT; // chi tiet bai thi, moi nut = id cau hoi + thu tu dap an da xao + dap an SV chon 

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
	string NoiDung;
    string DapAn[4];
	char DapAnDung;
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

#endif