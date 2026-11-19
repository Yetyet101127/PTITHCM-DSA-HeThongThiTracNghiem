#ifndef LOP_H
#define LOP_H

#include "KhaiBao.h"

const string FILE_LOP = "data/lop.txt";

void QuanLyLop(DsLOP &dsLop);

// tra ve chi so lop, -1 neu khong co; boQua: chi so bo qua (dung khi sua, tranh tu trung chinh no)
int TimLop(const DsLOP &dsLop, const string &maLop, int boQua = -1);

bool DocFileLop(DsLOP &dsLop, const string &tenFile);
bool GhiFileLop(const DsLOP &dsLop, const string &tenFile);

void NapDuLieu(DsLOP &dsLop);
bool LuuDuLieu(const DsLOP &dsLop);

void TaiLaiTuFile(DsLOP &dsLop);

void GiaiPhongDsLop(DsLOP &dsLop);

#endif