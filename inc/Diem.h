#ifndef DIEM_H
#define DIEM_H

#include "KhaiBao.h"

const string FILE_DIEM = "data/diem.txt";

string DinhDangDiem(float diem);

PTRDIEMTHI TimDiemThi(PTRSINHVIEN sv, const string &maMH);

void GiaiPhongDsDiem(PTRDIEMTHI &ds);

bool DocFileDiem(DsLOP &dsLop, const string &tenFile);
bool GhiFileDiem(const DsLOP &dsLop, const string &tenFile);

void DanhDauCauHoiDaThi(const DsLOP &dsLop, treeMH goc);

void InBangDiem(treeMH dsMonHoc, DsLOP &dsLop);

void InChiTietBaiThi(treeMH dsMonHoc, DsLOP &dsLop);

#endif