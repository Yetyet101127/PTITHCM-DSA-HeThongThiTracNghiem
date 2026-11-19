#include <ctime>
#include "mylib.h"
#include "UI.h"
#include "DangNhap.h"
#include "Lop.h"
#include "MonHoc.h"
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

static void VeThongTinSV(const DsLOP &dsLop, PTRSINHVIEN sv) {
    const int hang = 8, col = 75, rong = 60;
    const int le = 2, rongNhan = 16, rongTrong = rong - 2 * le;
    const int soDong = 5;
    const int hangCuoi = hang + 3 + soDong;   // vien duoi
    const int VANG = 14, TRANG = 15, XANH_DUONG = 9;

    LOP *lop = TimLopCuaSV(dsLop, sv);
    const string nhan[soDong] = { "Ho va ten:", "Ma sinh vien:", "Ma lop:", "Ten lop:", "Phai:"};
    const string giaTri[soDong] = { sv->data.HO + " " + sv->data.TEN, sv->data.MASV,
                                    lop ? lop->MALOP : "", lop ? lop->TENLOP : "" , sv->data.PHAI};

    VeDong(hang, col, string(rong, '-'), rong, 0, TRANG);
    for (int r = hang + 1; r < hangCuoi; r++) {
        VeDong(r, col, "|", 1, 0, TRANG);
        VeDong(r, col + rong - 1, "|", 1, 0, TRANG);
    }
    VeDong(hang + 2, col, string(rong, '-'), rong, 0, TRANG);
    VeDong(hangCuoi, col, string(rong, '-'), rong, 0, TRANG);

    VeDong(hang + 1, col + le, CanGiua("THONG TIN SINH VIEN", rongTrong), rongTrong, 0, VANG);
    for (int i = 0; i < soDong; i++) {
        int hangDong = hang + 3 + i;
        VeDong(hangDong, col + le, nhan[i], rongNhan, 0, XANH_DUONG);
        VeDong(hangDong, col + le + rongNhan, RutGon(giaTri[i], rongTrong - rongNhan), rongTrong - rongNhan, 0, TRANG);
    }
}

static void MenuSV(treeMH &dsMonHoc, DsLOP &dsLop, PTRSINHVIEN svDangNhap) {
    string dsChucNang[] = { "1. Thi Trac Nghiem", "2. Dang xuat" };
    const int soLuong = 2;
    const string phuDe = "Ban duoc cap quyen: Sinh vien";

    // con bai thi dang do (cup dien, tat chuong trinh,...) thi vao thang bai thi do, thi xong vao man hinh chon mon
    if (TiepTucBaiThiDangDo(dsMonHoc, dsLop, svDangNhap)) ThiTracNghiem(dsMonHoc, dsLop, svDangNhap);

    VeNenManHinh(phuDe);
    VeThongTinSV(dsLop, svDangNhap);
    int luaChon = 0;
    while (true) {
        luaChon = MenuTong(dsChucNang, soLuong, 8, 5, luaChon);
        if (luaChon == 1) {
            if (XacNhan("Ban co muon dang xuat?")) break;
            continue;
        }
        if (luaChon == 0) ThiTracNghiem(dsMonHoc, dsLop, svDangNhap);
        VeNenManHinh(phuDe);
        VeThongTinSV(dsLop, svDangNhap);
    }
}

int main() {
    srand((unsigned)time(NULL));
    treeMH dsMonHoc = nullptr;
    DsLOP  dsLop;
    NapDuLieu(dsLop);
    NapMonHoc(dsMonHoc, dsLop);
    XuLyBaiThiHetHan(dsMonHoc, dsLop);   // phien thi da het gio luc tat may -> tu cham diem

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