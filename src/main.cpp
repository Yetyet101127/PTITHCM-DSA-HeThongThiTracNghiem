#include "mylib.h"
#include "UI.h"
#include "DangNhap.h"
#include "Lop.h"
#include "MonHoc.h"
#include "CauHoi.h"
#include "SinhVien.h"
#include "Thi.h"
#include "Diem.h"

using namespace std;

static void MenuGV(treeMH &dsMonHoc, DsLOP &dsLop) {
    string dsChucNang[] = { "1. Quan ly lop", "2. Quan ly mon hoc", "3. In bang diem", "4. In chi tiet bai thi", "5. Dang xuat" };
    const int soLuong = 5;
    const string phuDe = "Ban duoc cap quyen: Giang vien";

    VeNenManHinh(phuDe);
    int luaChon = 0;
    while (true) {
        luaChon = MenuTong(dsChucNang, soLuong, 8, 5, luaChon);
        if (luaChon == 4) {
            if (XacNhan("Ban co muon dang xuat?")) break;
            continue;
        }
        else if (luaChon == 0) QuanLyLop(dsLop);
        else if (luaChon == 1) QuanLyMonHoc(dsMonHoc, dsLop);
        else if (luaChon == 2) InBangDiem(dsMonHoc, dsLop);
        else if (luaChon == 3) InChiTietBaiThi(dsMonHoc, dsLop);
        VeNenManHinh(phuDe);
    }
}

static void MenuSV(treeMH &dsMonHoc, DsLOP &dsLop, PTRSINHVIEN svDangNhap) {
    string dsChucNang[] = { "1. Thi Trac Nghiem", "2. Dang xuat" };
    const int soLuong = 2;
    const string phuDe = "Ban duoc cap quyen: Sinh vien";

    VeNenManHinh(phuDe);
    int luaChon = 0;
    while (true) {
        luaChon = MenuTong(dsChucNang, soLuong, 8, 5, luaChon);
        if (luaChon == 1) {
            if (XacNhan("Ban co muon dang xuat?")) break;
            continue;
        }
        if (luaChon == 0) ThiTracNghiem(dsMonHoc, dsLop, svDangNhap);
        VeNenManHinh(phuDe);
    }
}

int main() {
    treeMH dsMonHoc = nullptr;
    DsLOP  dsLop;
    NapDuLieu(dsLop);
    NapMonHoc(dsMonHoc, dsLop);   

    KhoiTaoConsole();
    AnHienConTro(false);

    while (true) {
        PTRSINHVIEN svDangNhap = nullptr;
        VaiTro vt = DangNhap(dsLop, svDangNhap);

        if (vt == THOAT) break;
        if (vt == vaitroGV) MenuGV(dsMonHoc, dsLop);
        else if (vt == vaitroSV) MenuSV(dsMonHoc, dsLop, svDangNhap);
    }

    GiaiPhongCayMonHoc(dsMonHoc);
    GiaiPhongDsLop(dsLop);
    return 0;
}