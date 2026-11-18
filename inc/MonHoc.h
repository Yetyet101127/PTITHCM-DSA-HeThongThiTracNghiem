#ifndef MONHOC_H
#define MONHOC_H

#include "KhaiBao.h"

const string FILE_MONHOC = "data/monhoc.txt";

struct MangMonHoc {
    treeMH *ds = nullptr;
    int soLuong = 0;
    int sucChua = 0;
};

void DamBaoSucChuaMang(MangMonHoc &m, int can);
void DongBoMangMonHoc(MangMonHoc &m, treeMH goc);   // nap lai theo LNR
void GiaiPhongMangMonHoc(MangMonHoc &m);

void QuanLyMonHoc(treeMH &dsMonHoc, DsLOP &dsLop);

treeMH TimMonHoc(treeMH goc, const string &maMH);

// duyet LNR: ds theo ma tang dan
int DemMonHoc(treeMH goc);
void LietKeMonHoc(treeMH goc, treeMH ds[], int &i);

bool DocFileMonHoc(treeMH &goc, const string &tenFile);
bool GhiFileMonHoc(treeMH goc, const string &tenFile);

// nap mon hoc + cau hoi (goi sau NapDuLieu)
void NapMonHoc(treeMH &goc, const DsLOP &dsLop);
bool LuuMonHoc(treeMH goc);
void TaiLaiMonHoc(treeMH &goc, const DsLOP &dsLop);
void GiaiPhongCayMonHoc(treeMH &goc);

#endif