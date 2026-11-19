#ifndef CAUHOI_H
#define CAUHOI_H

#include "KhaiBao.h"

const string FILE_CAUHOI = "data/cauhoi.txt";

// true neu da ghi/nap lai file (noi goi phai dong bo lai mang mon hoc)
bool QuanLyCauHoi(treeMH &dsMonHoc, DsLOP &dsLop, const string &maMH);

int LayIdCauHoiMoi();   // lay id moi va tang bo dem
int XemIdCauHoiMoi();   // chi xem id ke tiep, khong tang

bool DocFileCauHoi(treeMH goc, const string &tenFile);
bool GhiFileCauHoi(treeMH goc, const string &tenFile);

#endif