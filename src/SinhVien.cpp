#include "mylib.h"
#include "SinhVien.h"
#include "Lop.h"
#include "Diem.h"
#include "UI.h"
using namespace std;

PTRSINHVIEN TimSinhVien(const DsLOP &dsLop, const string &masv) {
    for (int i = 0; i < dsLop.n; i++) {
        for (PTRSINHVIEN p = dsLop.nodes[i]->Dssv; p != nullptr; p = p->next)
            if (p->data.MASV == masv) return p;
    }
    return nullptr;
}

LOP* TimLopCuaSV(const DsLOP &dsLop, PTRSINHVIEN sv) {
    for (int i = 0; i < dsLop.n; i++)
        for (PTRSINHVIEN p = dsLop.nodes[i]->Dssv; p != nullptr; p = p->next)
            if (p == sv) return dsLop.nodes[i];
    return nullptr;
}

void GiaiPhongDsSV(PTRSINHVIEN &ds) {
    for (PTRSINHVIEN p = ds; p != nullptr; p = p->next) GiaiPhongDsDiem(p->data.Diem);
    GiaiPhongDs(ds);
}

static void ThemSinhVien(LOP *lop, const SINHVIEN &sv) {
    PTRSINHVIEN p = new NodeSINHVIEN();
    p->data = sv;
    ThemCuoi(lop->Dssv, p);
}

static void XoaSinhVien(LOP *lop, PTRSINHVIEN p) {
    GoNut(lop->Dssv, p);
    delete p;
}

// MALOP|MASV|HO|TEN|PHAI|MATKHAU
bool DocFileSinhVien(DsLOP &dsLop, const string &tenFile) {
    return DocFileTheoDong(tenFile, [&dsLop](string t[], int n) {
        if (n < 5) return;
        int vtLop = TimLop(dsLop, VietHoa(TrimChuoi(t[0])));
        SINHVIEN sv;
        sv.MASV = VietHoa(TrimChuoi(t[1]));
        sv.HO = t[2];
        sv.TEN = t[3];
        sv.PHAI = t[4];
        string mk = (n >= 6) ? TrimChuoi(t[5]) : "";
        if (!mk.empty()) sv.MATKHAU = mk;   // thieu hoac rong mat khau: giu mac dinh MKSV
        if (vtLop == -1 || sv.MASV.empty() || sv.HO.empty() || sv.TEN.empty()) return;
        if (TimSinhVien(dsLop, sv.MASV) != nullptr) return;
        ThemSinhVien(dsLop.nodes[vtLop], sv);
    });
}

bool GhiFileSinhVien(const DsLOP &dsLop, const string &tenFile) {
    ofstream f(tenFile.c_str());
    if (!f.is_open()) return false;

    for (int i = 0; i < dsLop.n; i++)
        for (PTRSINHVIEN p = dsLop.nodes[i]->Dssv; p != nullptr; p = p->next)
            f << dsLop.nodes[i]->MALOP << "|" << p->data.MASV << "|" << p->data.HO << "|"
              << p->data.TEN << "|" << p->data.PHAI << "|" << p->data.MATKHAU << "\n";
    return true;
}

// so sanh cac truong sua duoc
static bool KhacNhau(const SINHVIEN &x, const SINHVIEN &y) {
    return x.MASV != y.MASV || x.HO != y.HO || x.TEN != y.TEN || x.PHAI != y.PHAI;
}

class NghiepVuSinhVien : public IManHinhNhap {
private:
    DsLOP &goc;
    string maLop;   // luu ma thay vi con tro: TaiLaiTuFile giai phong het LOP, con tro cu se treo
    PTRSINHVIEN snap;

    LOP* Lop() const {
        int i = TimLop(goc, maLop);
        return (i == -1) ? nullptr : goc.nodes[i];
    }

    PTRSINHVIEN DanhSach() const {
        LOP *lop = Lop();
        return lop ? lop->Dssv : nullptr;
    }

    PTRSINHVIEN Nut(int vt) const { return NutThuI(DanhSach(), vt); }

    PTRSINHVIEN TimTrongSnap(const string &masv) const {
        return TimTrongDs(snap, [](PTRSINHVIEN p) { return p->data.MASV; }, masv);
    }

public:
    NghiepVuSinhVien(DsLOP &ds, const string &ma) : goc(ds), maLop(ma), snap(nullptr) {
        CapNhatSnapshot();
    }
    ~NghiepVuSinhVien() { GiaiPhongDsSV(snap); }

    int LayTongSo() override { return DemNut(DanhSach()); }

    void LayDuLieuBang(int vt, string cot[], TrangThaiDong &trangThai) override {
        trangThai = DONG_BINH_THUONG;
        PTRSINHVIEN p = Nut(vt);
        if (!p) return;
        cot[0] = SoSangChuoi(vt + 1);
        cot[1] = p->data.MASV;
        cot[2] = p->data.HO;
        cot[3] = p->data.TEN;
        cot[4] = p->data.PHAI;

        if (p->data.Diem != nullptr) { trangThai = DONG_KHOA; return; }

        PTRSINHVIEN s = TimTrongSnap(p->data.MASV);
        if (s == nullptr || KhacNhau(s->data, p->data)) trangThai = DONG_CHUA_LUU;
    }

    void LayDuLieuTimKiem(int vt, string cot[]) override {
        PTRSINHVIEN p = Nut(vt);
        if (!p) return;
        cot[2] = p->data.HO + " " + p->data.TEN;   // ghep ho + ten de tim theo ten day du
    }

    void LayDuLieuForm(int vt, string giaTri[]) override {
        PTRSINHVIEN p = Nut(vt);
        if (!p) return;
        giaTri[0] = p->data.MASV;
        giaTri[1] = p->data.HO;
        giaTri[2] = p->data.TEN;
        giaTri[3] = p->data.PHAI;
    }

    string KiemTraNghiepVu(const string gt[], PTRSINHVIEN dangSua) {
        bool doiMa = (dangSua == nullptr || gt[0] != dangSua->data.MASV);
        bool trung = doiMa && TimSinhVien(goc, gt[0]) != nullptr;
        return KiemTraTrungKhoa(gt, 4, trung, "Ma sinh vien");
    }

    string KiemTraTruocSua(int vt) override {
        PTRSINHVIEN p = Nut(vt);
        if (p == nullptr) return "";
        if (p->data.Diem != nullptr) return "Sinh vien da du thi, khong the sua hoac xoa!";
        if (SinhVienDangThi(p->data.MASV)) return "Sinh vien dang co bai thi do dang, khong the sua hoac xoa!";
        return "";
    }

    string XuLyThem(const string gt[]) override {
        LOP *lop = Lop();
        if (!lop) return "Khong tim thay lop!";
        string loi = KiemTraNghiepVu(gt, nullptr);
        if (!loi.empty()) return loi;

        SINHVIEN sv;
        sv.MASV = gt[0];
        sv.HO = gt[1];
        sv.TEN = gt[2];
        sv.PHAI = gt[3];
        sv.MATKHAU = MKSV;
        ThemSinhVien(lop, sv);
        return "";
    }

    string XuLySua(int vt, const string gt[]) override {
        PTRSINHVIEN p = Nut(vt);
        if (!p) return "Khong tim thay sinh vien!";
        string loi = KiemTraTruocSua(vt);
        if (loi.empty()) loi = KiemTraNghiepVu(gt, p);
        if (!loi.empty()) return loi;

        p->data.MASV = gt[0];
        p->data.HO = gt[1];
        p->data.TEN = gt[2];
        p->data.PHAI = gt[3];
        return "";
    }

    string XuLyXoa(int vt) override {
        PTRSINHVIEN p = Nut(vt);
        if (!p) return "Khong tim thay sinh vien!";

        string loi = KiemTraTruocSua(vt);
        if (!loi.empty()) return loi;
        XoaSinhVien(Lop(), p);
        return "";
    }

    bool XuLyLuu() override {
        return LuuDuLieu(goc);
    }

    bool CoThayDoi() override {
        return DsKhacNhau(DanhSach(), snap, [](PTRSINHVIEN x, PTRSINHVIEN y) { return KhacNhau(x->data, y->data); });
    }

    void KhoiPhucSnapshot() override { TaiLaiTuFile(goc); }

    void CapNhatSnapshot() override {
        GiaiPhongDsSV(snap);
        snap = SaoChepDs(DanhSach(), [](PTRSINHVIEN p) {
            PTRSINHVIEN m = new NodeSINHVIEN();
            m->data = p->data;
            m->data.Diem = nullptr;   // khong chia se ds diem
            return m;
        });
    }
};

void QuanLySinhVien(DsLOP &dsLop, LOP *lop) {
    if (!lop) return;

    DuLieuManHinhNhap data;
    data.tieuDe      = "Danh sach Sinh vien - Lop: " + lop->MALOP;
    data.batDauCheDoCho = true;
    data.coTimKiem = true;
    data.cotTimKiem = 2;
    data.tenTimKiem = "ho ten";
    data.loaiTimKiem = TRUONG_CHU;

    KhoiTaoBang(data.bang, HANG_BANG, COT_BANG_TRAI, "sinh vien");
    ThemCot(data.bang, "STT", 5);
    ThemCot(data.bang, "MA SV", 14);
    ThemCot(data.bang, "HO VA TEN DEM", 29);
    ThemCot(data.bang, "TEN", 12);
    ThemCot(data.bang, "PHAI", 8);

    data.soTruongForm = 5;
    data.nhanForm[0] = "Ma SV:";          data.loaiTruong[0] = TRUONG_MA;
    data.nhanForm[1] = "Ho va ten dem:";  data.loaiTruong[1] = TRUONG_HODEM;
    data.nhanForm[2] = "Ten:";            data.loaiTruong[2] = TRUONG_TEN;
    data.nhanForm[3] = "Phai:";           data.loaiTruong[3] = TRUONG_PHAI;
    data.nhanForm[4] = "XAC NHAN";        data.loaiTruong[4] = TRUONG_NUT;
    KhoiTaoTrangThai(data.tt);

    NghiepVuSinhVien nv(dsLop, lop->MALOP);
    data.nghiepVu = &nv;

    ChayManHinhNhap(data);
}