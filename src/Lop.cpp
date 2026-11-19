#include "mylib.h"
#include "Lop.h"
#include "SinhVien.h"
#include "Diem.h"
#include "UI.h"
using namespace std;

int TimLop(const DsLOP &dsLop, const string &maLop, int boQua) {
    for (int i = 0; i < dsLop.n; i++) {
        if (i == boQua) continue;
        if (dsLop.nodes[i]->MALOP == maLop) return i;
    }
    return -1;
}

bool DocFileLop(DsLOP &dsLop, const string &tenFile) {
    return DocFileTheoDong(tenFile, [&dsLop](string t[], int n) {
        if (n < 2 || dsLop.n >= MAX_LOP) return;
        string ma  = VietHoa(TrimChuoi(t[0]));
        string ten = TrimChuoi(t[1]);
        if (ma.empty() || ten.empty() || TimLop(dsLop, ma) != -1) return;
        LOP *l = new LOP();
        l->MALOP = ma;
        l->TENLOP = ten;
        dsLop.nodes[dsLop.n++] = l;
    });
}

bool GhiFileLop(const DsLOP &dsLop, const string &tenFile) {
    ofstream f(tenFile.c_str());
    if (!f.is_open()) return false;
    for (int i = 0; i < dsLop.n; i++)
        f << dsLop.nodes[i]->MALOP << "|" << dsLop.nodes[i]->TENLOP << "\n";
    return true;
}

void NapDuLieu(DsLOP &dsLop) {
    DocFileLop(dsLop, FILE_LOP);
    DocFileSinhVien(dsLop, FILE_SINHVIEN);
    DocFileDiem(dsLop, FILE_DIEM);
}

bool LuuDuLieu(const DsLOP &dsLop) {
    bool a = GhiFileLop(dsLop, FILE_LOP);
    bool b = GhiFileSinhVien(dsLop, FILE_SINHVIEN);
    bool c = GhiFileDiem(dsLop, FILE_DIEM);
    return a && b && c;
}

void GiaiPhongDsLop(DsLOP &ds) {
    for (int i = 0; i < ds.n; i++) {
        GiaiPhongDsSV(ds.nodes[i]->Dssv);
        delete ds.nodes[i];
        ds.nodes[i] = nullptr;
    }
    ds.n = 0;
}

void TaiLaiTuFile(DsLOP &ds) {
    GiaiPhongDsLop(ds);
    NapDuLieu(ds);
}

struct MucLop {
    string ma, ten;
};

class NghiepVuLop : public IManHinhNhap {
private:
    DsLOP &goc;
    MucLop *snap;
    int soSnap;

public:
    NghiepVuLop(DsLOP &ds) : goc(ds), snap(nullptr), soSnap(0) { CapNhatSnapshot(); }
    ~NghiepVuLop() { delete[] snap; }

    int LayTongSo() override { return goc.n; }

    void LayDuLieuBang(int vt, string cot[], TrangThaiDong &trangThai) override {
        LOP *l = goc.nodes[vt];
        cot[0] = SoSangChuoi(vt + 1);
        cot[1] = l->MALOP;
        cot[2] = l->TENLOP;
        cot[3] = SoSangChuoi(DemNut(l->Dssv));
        trangThai = DONG_BINH_THUONG;

        if (l->Dssv != nullptr) { trangThai = DONG_KHOA; return; }

        const MucLop *m = TimTrongMang(snap, soSnap, [](const MucLop &x) { return x.ma; }, l->MALOP);
        if (m == nullptr || m->ten != l->TENLOP) trangThai = DONG_CHUA_LUU;
    }

    void LayDuLieuForm(int vt, string giaTri[]) override {
        giaTri[0] = goc.nodes[vt]->MALOP;
        giaTri[1] = goc.nodes[vt]->TENLOP;
    }

    string KiemTraNghiepVu(const string gt[], int vtSua) {
        return KiemTraTrungKhoa(gt, 2, TimLop(goc, gt[0], vtSua) != -1, "Ma lop");
    }

    string XuLyThem(const string gt[]) override {
        if (goc.n >= MAX_LOP) return "Danh sach lop da day!";
        string loi = KiemTraNghiepVu(gt, -1);
        if (!loi.empty()) return loi;

        LOP *l = new LOP();
        l->MALOP = gt[0];
        l->TENLOP = gt[1];
        goc.nodes[goc.n++] = l;
        return "";
    }

    string KiemTraTruocSua(int vt) override {
        return (goc.nodes[vt]->Dssv != nullptr) ? "Lop da co sinh vien, khong the sua hoac xoa!" : "";
    }

    string XuLySua(int vt, const string gt[]) override {
        string loi = KiemTraTruocSua(vt);
        if (loi.empty()) loi = KiemTraNghiepVu(gt, vt);
        if (!loi.empty()) return loi;

        goc.nodes[vt]->MALOP = gt[0];
        goc.nodes[vt]->TENLOP = gt[1];
        return "";
    }

    string XuLyXoa(int vt) override {
        string loi = KiemTraTruocSua(vt);
        if (!loi.empty()) return loi;

        delete goc.nodes[vt];
        XoaPhanTuMang(goc.nodes, goc.n, vt);
        return "";
    }

    bool XuLyLuu() override {
        return LuuDuLieu(goc);
    }

    bool CoThayDoi() override {
        return MangKhacNhau(goc.nodes, goc.n, snap, soSnap,
            [](LOP *l, const MucLop &s) { return l->MALOP != s.ma || l->TENLOP != s.ten; });
    }

    void KhoiPhucSnapshot() override { TaiLaiTuFile(goc); }

    void CapNhatSnapshot() override {
        SaoChepMang(snap, soSnap, goc.nodes, goc.n,
            [](LOP *l) { return MucLop{ l->MALOP, l->TENLOP }; });
    }

    void XuLyEnter(int vt) override {
        QuanLySinhVien(goc, goc.nodes[vt]);
        CapNhatSnapshot();   // man hinh SV co the da luu hoac nap lai file
    }
};

void QuanLyLop(DsLOP &dsLop) {
    DuLieuManHinhNhap data;
    data.tieuDe = "Danh sach Lop";
    data.huongDanXem = "1: Nhap lop   Enter: Nhap DSSV";

    data.coTimKiem = true;
    data.cotTimKiem = 2;
    data.tenTimKiem = "ten lop";
    data.loaiTimKiem = TRUONG_CHU;

    KhoiTaoBang(data.bang, HANG_BANG, COT_BANG_TRAI, "lop");
    ThemCot(data.bang, "STT", 5);
    ThemCot(data.bang, "MA LOP", 15);
    ThemCot(data.bang, "TEN LOP", 40);
    ThemCot(data.bang, "SO SV", 8);

    data.soTruongForm = 3;
    data.nhanForm[0] = "Ma lop:"; data.loaiTruong[0] = TRUONG_MA;
    data.nhanForm[1] = "Ten lop:"; data.loaiTruong[1] = TRUONG_TENLOP;
    data.nhanForm[2] = "XAC NHAN"; data.loaiTruong[2] = TRUONG_NUT;
    KhoiTaoTrangThai(data.tt);

    NghiepVuLop nv(dsLop);
    data.nghiepVu = &nv;

    ChayManHinhNhap(data);
}



