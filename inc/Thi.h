#ifndef THI_H
#define THI_H

#include "KhaiBao.h"

const int TOI_DA_PHUT_THI = 180;

void ThiTracNghiem(treeMH &dsMonHoc, DsLOP &dsLop, PTRSINHVIEN svDangNhap);
// GV xem chi tiet bai thi cua sv o mon mh (chi xem)
void XemChiTietBaiThi(treeMH gocMH, PTRSINHVIEN sv, treeMH mh);

// xem chi tiet cau hoi cua mon (giao dien giong man hinh thi), viTriBatDau: cau hien ra dau tien (tinh tu 0)
void XemChiTietCauHoi(treeMH mh, int viTriBatDau);

// goi 1 lan luc khoi dong (sau NapMonHoc): phien thi da qua gio ket thuc -> tu cham, luu diem
void XuLyBaiThiHetHan(treeMH goc, DsLOP &dsLop);

// goi khi sinh vien dang nhap: neu con bai thi dang do thi vao thang bai thi do. true neu co bai thi dang do
bool TiepTucBaiThiDangDo(treeMH goc, DsLOP &dsLop, PTRSINHVIEN sv);

#endif