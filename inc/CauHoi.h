#ifndef CAUHOI_H
#define CAUHOI_H

#include "KhaiBao.h"

const string FILE_CAUHOI = "data/cauhoi.txt";

bool QuanLyCauHoi(treeMH &dsMonHoc, DsLOP &dsLop, const string &maMH);

int LayIdCauHoiMoi();
int XemIdCauHoiMoi();

bool DocFileCauHoi(treeMH goc, const string &tenFile);
bool GhiFileCauHoi(treeMH goc, const string &tenFile);

#endif