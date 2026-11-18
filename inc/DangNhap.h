#ifndef DANGNHAP_H
#define DANGNHAP_H

#include "KhaiBao.h"

enum VaiTro { vaitroGV, vaitroSV, THOAT };
VaiTro DangNhap(DsLOP &dsLop, PTRSINHVIEN &svDangNhap);

#endif