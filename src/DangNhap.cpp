#include "mylib.h"
#include "DangNhap.h"
#include "UI.h"
#include "SinhVien.h"
using namespace std;

enum { F_TK, F_MK, F_DN, SO_TRUONG };

VaiTro DangNhap(DsLOP &dsLop, PTRSINHVIEN &svDangNhap) {
    svDangNhap = nullptr;

    Truong form[SO_TRUONG] = {
        {TRUONG_DANGNHAP, "Tai khoan", "", 9,  10, 50},
        {TRUONG_MATKHAU, "Mat khau",  "", 14, 10, 50},
        {TRUONG_NUT,     "DANG NHAP", "", 19 , 10, 50}
    };

    VeNenManHinh("Dang Nhap He Thong");
    VeDong(25, 10, "(Nhan Esc de thoat chuong trinh)", 40, 0, 8);

    int focus = F_TK;
    string loi;
    while (true) {
        int chon = ChayForm(form, SO_TRUONG, focus, loi);
        if (chon == -1) return THOAT;

        const string &tk = form[F_TK].giaTri;
        const string &mk = form[F_MK].giaTri;

        if (tk.empty() || mk.empty()) {
            loi = "Vui long nhap day du tai khoan va mat khau!";
        }
        else if (tk == "GV" && mk == "GV") {   // tai khoan giang vien co dinh
            return vaitroGV;
        }
        else {
            PTRSINHVIEN sv = TimSinhVien(dsLop, tk);
            if (sv == nullptr) {
                loi = "Dang nhap that bai! Khong tim thay tai khoan.";
            }
            else if (sv->data.MATKHAU == mk) {
                svDangNhap = sv;
                return vaitroSV;
            }
            else {
                loi = "Dang nhap that bai! Sai mat khau.";
            }
        }
    }
}