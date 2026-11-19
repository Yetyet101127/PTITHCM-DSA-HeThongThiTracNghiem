#ifndef DIEM_H
#define DIEM_H

#include <functional>
#include "KhaiBao.h"

const string FILE_DIEM = "data/diem.txt";
const string FILE_DANGTHI = "data/dangthi.txt";

string DinhDangDiem(float diem);

PTRDIEMTHI TimDiemThi(PTRSINHVIEN sv, const string &maMH);

void GiaiPhongDsDiem(PTRDIEMTHI &ds);

// chi tiet bai thi <-> chuoi "id:hoanvi:chon;..." vd "12:3142:A;7:1234:-"
// (hoanvi = thu tu dap an da xao, '-' = bo trong)
void NapChiTiet(PTRCTBT &dau, const string &s);
string ChiTietSangChuoi(PTRCTBT dau);

// PHIEN THI DANG DO: moi sinh vien dang thi co 1 dong trong dangthi.txt
// MALOP|MASV|MAMH|SOPHUT|KETTHUC|CHITIET   (KETTHUC = gio ket thuc (giay tu 1970), tinh san luc bat dau thi)
struct PhienThi {
    string maLop, maSV, maMH;
    int soPhut = 0;
    long long ketThuc = 0;
    PTRCTBT dsCTBT = nullptr;
};
bool GhiPhienThi(const PhienThi &p);                 // them moi hoac ghi de dong cua sinh vien
void XoaPhienThi(const string &maSV);
bool DocPhienThi(const string &maSV, PhienThi &kq);  // true neu co; nguoi goi GiaiPhongDs(kq.dsCTBT)
void DuyetPhienThi(std::function<void(PhienThi &)> xuLy);
bool SinhVienDangThi(const string &maSV);

bool DocFileDiem(DsLOP &dsLop, const string &tenFile);
bool GhiFileDiem(const DsLOP &dsLop, const string &tenFile);

void DanhDauCauHoiDaThi(const DsLOP &dsLop, treeMH goc);

void InBangDiem(treeMH dsMonHoc, DsLOP &dsLop);

void InChiTietBaiThi(treeMH dsMonHoc, DsLOP &dsLop);

#endif