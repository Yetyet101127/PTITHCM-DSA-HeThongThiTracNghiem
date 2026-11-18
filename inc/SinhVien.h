#ifndef SINHVIEN_H
#define SINHVIEN_H

#include "KhaiBao.h"

const string FILE_SINHVIEN = "data/sinhvien.txt";

void QuanLySinhVien(DsLOP &dsLop, LOP *lop);

PTRSINHVIEN TimSinhVien(const DsLOP &dsLop, const string &masv);

void GiaiPhongDsSV(PTRSINHVIEN &ds);

bool DocFileSinhVien(DsLOP &dsLop, const string &tenFile);
bool GhiFileSinhVien(const DsLOP &dsLop, const string &tenFile);

#endif
