#include "mylib.h"
#include "MonHoc.h"
#include "CauHoi.h"
#include "Diem.h"
#include "UI.h"
using namespace std;

void DamBaoSucChuaMang(MangMonHoc &m, int can) {
    if (can <= m.sucChua) return;
    int moi = (m.sucChua > 0) ? m.sucChua : 16;
    while (moi < can) moi *= 2;
    treeMH *mangMoi = new treeMH[moi];
    for (int i = 0; i < m.soLuong; i++) mangMoi[i] = m.ds[i];
    delete[] m.ds;
    m.ds = mangMoi;
    m.sucChua = moi;
}

treeMH TimMonHoc(treeMH goc, const string &maMH) {
    while (goc != nullptr) {
        if (maMH == goc->data.MAMH) return goc;
        goc = (maMH < goc->data.MAMH) ? goc->left : goc->right;
    }
    return nullptr;
}

static int DemMonHoc(treeMH goc) {
    return goc == nullptr ? 0 : 1 + DemMonHoc(goc->left) + DemMonHoc(goc->right);
}

// duyet LNR: ds theo ma tang dan
static void LietKeMonHoc(treeMH goc, treeMH ds[], int &i) {
    if (goc == nullptr) return;
    LietKeMonHoc(goc->left, ds, i);
    ds[i++] = goc;
    LietKeMonHoc(goc->right, ds, i);
}

void DongBoMangMonHoc(MangMonHoc &m, treeMH goc) {
    m.soLuong = 0;
    DamBaoSucChuaMang(m, DemMonHoc(goc));
    LietKeMonHoc(goc, m.ds, m.soLuong);
}

void GiaiPhongMangMonHoc(MangMonHoc &m) {
    delete[] m.ds;
    m.ds = nullptr;
    m.soLuong = m.sucChua = 0;
}

static void ChenBST(treeMH &goc, treeMH nut) {
    treeMH *p = &goc;
    while (*p != nullptr)
        p = (nut->data.MAMH < (*p)->data.MAMH) ? &(*p)->left : &(*p)->right;
    *p = nut;
}

// go nut co ma = ma khoi cay, tra ve nut do (chua delete); nut co 2 con thi thay bang nut trai nhat cua cay con phai
static treeMH GoBST(treeMH &goc, const string &ma) {
    treeMH *p = &goc;
    while (*p != nullptr && (*p)->data.MAMH != ma)
        p = (ma < (*p)->data.MAMH) ? &(*p)->left : &(*p)->right;
    if (*p == nullptr) return nullptr;

    treeMH x = *p;
    if (x->left == nullptr)       *p = x->right;
    else if (x->right == nullptr) *p = x->left;
    else {
        treeMH *q = &x->right;
        while ((*q)->left != nullptr) q = &(*q)->left;
        treeMH s = *q;
        *q = s->right;
        s->left = x->left;
        s->right = x->right;
        *p = s;
    }
    x->left = x->right = nullptr;
    return x;
}

static void XoaCay(treeMH n) {
    if (n == nullptr) return;
    XoaCay(n->left);
    XoaCay(n->right);
    GiaiPhongDs(n->data.dsCHT);
    delete n;
}

void GiaiPhongCayMonHoc(treeMH &goc) {
    XoaCay(goc);
    goc = nullptr;
}

bool DocFileMonHoc(treeMH &goc, const string &tenFile) {
    return DocFileTheoDong(tenFile, [&goc](string t[], int n) {
        if (n < 2) return;
        string ma  = VietHoa(TrimChuoi(t[0]));
        string ten = TrimChuoi(t[1]);
        if (ma.empty() || ten.empty() || TimMonHoc(goc, ma) != nullptr) return;
        treeMH nut = new Node_MONHOC();
        nut->data.MAMH = ma;
        nut->data.TENMH = ten;
        ChenBST(goc, nut);
    });
}

static void GhiCayMonHoc(treeMH n, ofstream &f) {
    if (n == nullptr) return;
    GhiCayMonHoc(n->left, f);
    f << n->data.MAMH << "|" << n->data.TENMH << "\n";
    GhiCayMonHoc(n->right, f);
}

bool GhiFileMonHoc(treeMH goc, const string &tenFile) {
    ofstream f(tenFile.c_str());
    if (!f.is_open()) return false;
    GhiCayMonHoc(goc, f);
    return true;
}

void NapMonHoc(treeMH &goc, const DsLOP &dsLop) {
    DocFileMonHoc(goc, FILE_MONHOC);
    DocFileCauHoi(goc, FILE_CAUHOI);
    DanhDauCauHoiDaThi(dsLop, goc);
}

bool LuuMonHoc(treeMH goc) {
    bool a = GhiFileMonHoc(goc, FILE_MONHOC);
    bool b = GhiFileCauHoi(goc, FILE_CAUHOI);
    return a && b;
}

void TaiLaiMonHoc(treeMH &goc, const DsLOP &dsLop) {
    GiaiPhongCayMonHoc(goc);
    NapMonHoc(goc, dsLop);
}

struct MucSnap {
    string ma, ten;
};

class NghiepVuMonHoc : public IManHinhNhap {
private:
    treeMH &goc;
    DsLOP &dsLop;
    MangMonHoc mang;   // thu tu hien thi
    MucSnap *snap;
    int soSnap;

    treeMH Nut(int vt) const { return (vt >= 0 && vt < mang.soLuong) ? mang.ds[vt] : nullptr; }

public:
    NghiepVuMonHoc(treeMH &g, DsLOP &ds) : goc(g), dsLop(ds), snap(nullptr), soSnap(0) {
        DongBoMangMonHoc(mang, goc);
        CapNhatSnapshot();
    }
    ~NghiepVuMonHoc() {
        delete[] snap;
        GiaiPhongMangMonHoc(mang);
    }

    int LayTongSo() override { return mang.soLuong; }

    void LayDuLieuBang(int vt, string cot[], TrangThaiDong &trangThai) override {
        trangThai = DONG_BINH_THUONG;
        treeMH nut = Nut(vt);
        if (nut == nullptr) return;
        cot[0] = SoSangChuoi(vt + 1);
        cot[1] = nut->data.MAMH;
        cot[2] = nut->data.TENMH;
        cot[3] = SoSangChuoi(DemNut(nut->data.dsCHT));

        if (nut->data.dsCHT != nullptr) { trangThai = DONG_KHOA; return; }
        const MucSnap *m = TimTrongMang(snap, soSnap, [](const MucSnap &x) { return x.ma; }, nut->data.MAMH);
        if (m == nullptr || m->ten != nut->data.TENMH) trangThai = DONG_CHUA_LUU;
    }

    void LayDuLieuForm(int vt, string giaTri[]) override {
        treeMH nut = Nut(vt);
        if (nut == nullptr) return;
        giaTri[0] = nut->data.MAMH;
        giaTri[1] = nut->data.TENMH;
    }

    string KiemTraNghiepVu(const string gt[], treeMH dangSua) {
        treeMH trung = TimMonHoc(goc, gt[0]);
        return KiemTraTrungKhoa(gt, 2, trung != nullptr && trung != dangSua, "Ma mon hoc");
    }

    string XuLyThem(const string gt[]) override {
        string loi = KiemTraNghiepVu(gt, nullptr);
        if (!loi.empty()) return loi;

        treeMH nut = new Node_MONHOC();
        nut->data.MAMH = gt[0];
        nut->data.TENMH = gt[1];
        ChenBST(goc, nut);
        DamBaoSucChuaMang(mang, mang.soLuong + 1);
        mang.ds[mang.soLuong++] = nut;
        return "";
    }

    string KiemTraTruocSua(int vt) override {
        treeMH nut = Nut(vt);
        if (nut == nullptr) return "Khong tim thay mon hoc!";
        return (nut->data.dsCHT != nullptr) ? "Mon hoc da co cau hoi, khong the sua hoac xoa!" : "";
    }

    string XuLySua(int vt, const string gt[]) override {
        string loi = KiemTraTruocSua(vt);
        if (!loi.empty()) return loi;
        treeMH nut = Nut(vt);
        loi = KiemTraNghiepVu(gt, nut);
        if (!loi.empty()) return loi;


        if (gt[0] != nut->data.MAMH) {   // doi khoa BST: go ra roi chen lai   
            GoBST(goc, nut->data.MAMH);
            nut->data.MAMH = gt[0];
            ChenBST(goc, nut);
        }
        nut->data.TENMH = gt[1];
        return "";
    }

    string XuLyXoa(int vt) override {
        string loi = KiemTraTruocSua(vt);
        if (!loi.empty()) return loi;

        treeMH nut = GoBST(goc, mang.ds[vt]->data.MAMH);
        XoaPhanTuMang(mang.ds, mang.soLuong, vt);
        delete nut;
        return "";
    }

    bool XuLyLuu() override {
        return LuuMonHoc(goc);
    }

    bool CoThayDoi() override {
        return MangKhacNhau(mang.ds, mang.soLuong, snap, soSnap,
            [](treeMH n, const MucSnap &s) { return n->data.MAMH != s.ma || n->data.TENMH != s.ten; });
    }

    void KhoiPhucSnapshot() override {
        TaiLaiMonHoc(goc, dsLop);
        DongBoMangMonHoc(mang, goc);   // cay vua nap lai: mang cu dang tro vao nut da giai phong   
    }

    void CapNhatSnapshot() override {
        SaoChepMang(snap, soSnap, mang.ds, mang.soLuong,
            [](treeMH n) { return MucSnap{ n->data.MAMH, n->data.TENMH }; });
    }

    void XuLyEnter(int vt) override {
        treeMH nut = Nut(vt);
        if (nut == nullptr) return;
        string ma = nut->data.MAMH;   // copy: QuanLyCauHoi co the nap lai cay, nut cu se treo   
        if (QuanLyCauHoi(goc, dsLop, ma)) {
            DongBoMangMonHoc(mang, goc);
            CapNhatSnapshot();
        }
    }
};

void QuanLyMonHoc(treeMH &dsMonHoc, DsLOP &dsLop) {
    DuLieuManHinhNhap data;
    data.tieuDe = "Danh sach Mon hoc";
    data.huongDanXem = "1: Nhap mon hoc   Enter: Nhap cau hoi";

    data.coTimKiem = true;
    data.cotTimKiem = 2;
    data.tenTimKiem = "ten mon hoc";
    data.loaiTimKiem = TRUONG_CHU;

    KhoiTaoBang(data.bang, HANG_BANG, COT_BANG_TRAI, "mon hoc");
    ThemCot(data.bang, "STT", 5);
    ThemCot(data.bang, "MA MH", 14);
    ThemCot(data.bang, "TEN MON HOC", 37);
    ThemCot(data.bang, "SO CAU HOI", 12);

    data.soTruongForm = 3;
    data.nhanForm[0] = "Ma mon hoc:";  data.loaiTruong[0] = TRUONG_MA;
    data.nhanForm[1] = "Ten mon hoc:"; data.loaiTruong[1] = TRUONG_TENLOP;
    data.nhanForm[2] = "XAC NHAN";     data.loaiTruong[2] = TRUONG_NUT;
    KhoiTaoTrangThai(data.tt);

    NghiepVuMonHoc nv(dsMonHoc, dsLop);
    data.nghiepVu = &nv;

    ChayManHinhNhap(data);
}