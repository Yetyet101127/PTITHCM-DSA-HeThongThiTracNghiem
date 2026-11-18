#include "mylib.h"
#include "CauHoi.h"
#include "MonHoc.h"
#include "UI.h"
using namespace std;

static const int TOI_DA_ID_CAUHOI = 1000000;
static int idKeTiep = 1;
int LayIdCauHoiMoi() { return idKeTiep++; }
int XemIdCauHoiMoi() { return idKeTiep; }

static void ThemCauHoi(MONHOC &mh, const CAUHOI &ch) {
    PTRCAUHOI p = new Node_CAUHOI();
    p->data = ch;
    ThemCuoi(mh.dsCHT, p);
}

bool DocFileCauHoi(treeMH goc, const string &tenFile) {
    idKeTiep = 1;
    bool *daCo = new bool[TOI_DA_ID_CAUHOI + 1]();  
    bool ok = DocFileTheoDong(tenFile, [goc, daCo](string t[], int n) {
        if (n < 8) return;
        treeMH mh = TimMonHoc(goc, VietHoa(TrimChuoi(t[0])));
        string dapAn = VietHoa(TrimChuoi(t[7]));
        CAUHOI ch;
        ch.id = atoi(t[1].c_str());
        ch.NoiDung = t[2];
        ch.a = t[3]; ch.b = t[4]; ch.c = t[5]; ch.d = t[6];

        if (mh == nullptr || ch.id <= 0 || ch.id > TOI_DA_ID_CAUHOI) return;
        if (dapAn.length() != 1 || dapAn[0] < 'A' || dapAn[0] > 'D') return;   
        if (daCo[ch.id]) return;
        daCo[ch.id] = true;
        ch.DapAn = dapAn[0];

        ThemCauHoi(mh->data, ch);
        if (ch.id >= idKeTiep) idKeTiep = ch.id + 1;
    });
    delete[] daCo;
    return ok;
}

static void GhiCay(treeMH n, ofstream &f) {
    if (n == nullptr) return;
    GhiCay(n->left, f);
    for (PTRCAUHOI p = n->data.dsCHT; p != nullptr; p = p->next)
        f << n->data.MAMH << "|" << p->data.id << "|" << p->data.NoiDung << "|"
          << p->data.a << "|" << p->data.b << "|" << p->data.c << "|" << p->data.d << "|"
          << p->data.DapAn << "\n";
    GhiCay(n->right, f);
}

bool GhiFileCauHoi(treeMH goc, const string &tenFile) {
    ofstream f(tenFile.c_str());
    if (!f.is_open()) return false;
    GhiCay(goc, f);
    return true;
}

static const string LOI_DA_THI = "Cau hoi da duoc su dung trong bai thi, khong the sua hoac xoa!";

static bool KhacNhau(const CAUHOI &x, const CAUHOI &y) {
    return x.id != y.id || x.NoiDung != y.NoiDung || x.a != y.a || x.b != y.b ||
           x.c != y.c || x.d != y.d || x.DapAn != y.DapAn;
}

static void GanTuForm(CAUHOI &ch, const string gt[]) {
    ch.NoiDung = ChuanHoaKhoangTrang(gt[1]);
    ch.a = ChuanHoaKhoangTrang(gt[2]);
    ch.b = ChuanHoaKhoangTrang(gt[3]);
    ch.c = ChuanHoaKhoangTrang(gt[4]);
    ch.d = ChuanHoaKhoangTrang(gt[5]);
    ch.DapAn = gt[6][0];
}

class NghiepVuCauHoi : public IManHinhNhap {
private:
    treeMH &goc;
    DsLOP &dsLop;
    string maMH;
    PTRCAUHOI snap;
    bool daDongBo;

    treeMH MH() const { return TimMonHoc(goc, maMH); }

    PTRCAUHOI Nut(int vt) const {
        treeMH mh = MH();
        return mh ? NutThuI(mh->data.dsCHT, vt) : nullptr;
    }

    PTRCAUHOI TimTrongSnap(int id) const {
        return TimTrongDs(snap, [](PTRCAUHOI p) { return p->data.id; }, id);
    }

public:
    NghiepVuCauHoi(treeMH &g, DsLOP &ds, const string &ma) : goc(g), dsLop(ds), maMH(ma), snap(nullptr), daDongBo(false) {
        CapNhatSnapshot();
    }
    ~NghiepVuCauHoi() { GiaiPhongDs(snap); }
    bool DaDongBoFile() const { return daDongBo; }

    int LayTongSo() override {
        treeMH mh = MH();
        return mh ? DemNut(mh->data.dsCHT) : 0;
    }

    void LayDuLieuBang(int vt, string cot[], TrangThaiDong &trangThai) override {
        trangThai = DONG_BINH_THUONG;
        PTRCAUHOI p = Nut(vt);
        if (p == nullptr) return;
        cot[0] = SoSangChuoi(vt + 1);
        cot[1] = SoSangChuoi(p->data.id);
        cot[2] = RutGon(p->data.NoiDung, 46);
        cot[3] = string(1, p->data.DapAn);

        if (p->data.DaThi) { trangThai = DONG_KHOA; return; }
        PTRCAUHOI cu = TimTrongSnap(p->data.id);
        if (cu == nullptr || KhacNhau(cu->data, p->data)) trangThai = DONG_CHUA_LUU;
    }

    void LayDuLieuTimKiem(int vt, string cot[]) override {
        PTRCAUHOI p = Nut(vt);
        if (p == nullptr) return;
        cot[2] = p->data.NoiDung;
    }

    void LayDuLieuForm(int vt, string giaTri[]) override {
        PTRCAUHOI p = Nut(vt);
        if (p == nullptr) return;
        giaTri[0] = SoSangChuoi(p->data.id);
        giaTri[1] = p->data.NoiDung;
        giaTri[2] = p->data.a; giaTri[3] = p->data.b;
        giaTri[4] = p->data.c; giaTri[5] = p->data.d;
        giaTri[6] = string(1, p->data.DapAn);
    }

    void LayDuLieuFormThem(string giaTri[]) override { giaTri[0] = SoSangChuoi(XemIdCauHoiMoi()); }
    bool TruongBiKhoaKhiThem(int idxTruong) override { return idxTruong == 0; }
    bool TruongBiKhoa(int vt, int idxTruong) override { (void)vt; return idxTruong == 0; }

    string KiemTraTruocSua(int vt) override {
        PTRCAUHOI p = Nut(vt);
        if (p != nullptr && p->data.DaThi) return LOI_DA_THI;
        return "";
    }

    string KiemTraNghiepVu(const string gt[]) {
        static const char *TEN_TRUONG[5] = { "noi dung cau hoi", "dap an A", "dap an B", "dap an C", "dap an D" };
        for (int i = 0; i < 5; i++)
            if (gt[i + 1].empty()) return string("Vui long nhap ") + TEN_TRUONG[i] + "!";

        string loi = KiemTraHopLeForm(gt + 1, 5);
        if (!loi.empty()) return loi;

        CAUHOI t;
        GanTuForm(t, gt);
        const string *da[4] = { &t.a, &t.b, &t.c, &t.d };
        for (int i = 0; i < 4; i++)
            for (int j = i + 1; j < 4; j++)
                if (*da[i] == *da[j]) return "Cac dap an khong duoc trung nhau!";
        return "";
    }

    string XuLyThem(const string gt[]) override {
        treeMH mh = MH();
        if (mh == nullptr) return "Khong tim thay mon hoc!";
        string loi = KiemTraNghiepVu(gt);
        if (!loi.empty()) return loi;
        if (XemIdCauHoiMoi() > TOI_DA_ID_CAUHOI) return "Da het ma cau hoi cho phep!";

        CAUHOI ch;
        ch.id = LayIdCauHoiMoi();
        GanTuForm(ch, gt);
        ThemCauHoi(mh->data, ch);
        return "";
    }

    string XuLySua(int vt, const string gt[]) override {
        PTRCAUHOI p = Nut(vt);
        if (p == nullptr) return "Khong tim thay cau hoi!";
        if (p->data.DaThi) return LOI_DA_THI;
        string loi = KiemTraNghiepVu(gt);
        if (!loi.empty()) return loi;

        GanTuForm(p->data, gt);
        return "";
    }

    string XuLyXoa(int vt) override {
        treeMH mh = MH();
        PTRCAUHOI p = Nut(vt);
        if (p == nullptr) return "Khong tim thay cau hoi!";
        if (p->data.DaThi) return LOI_DA_THI;

        GoNut(mh->data.dsCHT, p);
        delete p;
        return "";
    }

    bool XuLyLuu() override {
        bool ok = LuuMonHoc(goc);
        if (ok) daDongBo = true;
        return ok;
    }

    bool CoThayDoi() override {
        treeMH mh = MH();
        return DsKhacNhau(mh ? mh->data.dsCHT : nullptr, snap,
                          [](PTRCAUHOI x, PTRCAUHOI y) { return KhacNhau(x->data, y->data); });
    }

    void KhoiPhucSnapshot() override {
        TaiLaiMonHoc(goc, dsLop);
        daDongBo = true;
    }

    void CapNhatSnapshot() override {
        GiaiPhongDs(snap);
        treeMH mh = MH();
        snap = SaoChepDs(mh ? mh->data.dsCHT : nullptr, [](PTRCAUHOI p) {
            PTRCAUHOI m = new Node_CAUHOI();
            m->data = p->data;
            return m;
        });
    }
};

bool QuanLyCauHoi(treeMH &dsMonHoc, DsLOP &dsLop, const string &maMH) {
    treeMH mh = TimMonHoc(dsMonHoc, maMH);
    if (mh == nullptr) return false;

    DuLieuManHinhNhap data;
    data.tieuDe     = "Danh sach Cau hoi - Mon: " + mh->data.MAMH;
    data.batDauCheDoCho = true;
    data.coTimKiem = true;
    data.cotTimKiem = 2;
    data.tenTimKiem = "noi dung cau hoi";
    data.loaiTimKiem = TRUONG_CHU;
    data.dongMoiTrang = 20;

    KhoiTaoBang(data.bang, HANG_BANG, COT_BANG_TRAI, "cau hoi");
    ThemCot(data.bang, "STT", 5);
    ThemCot(data.bang, "ID", 7);
    ThemCot(data.bang, "NOI DUNG CAU HOI", 48);
    ThemCot(data.bang, "DAP AN", 8);

    data.soTruongForm = 8;
    data.nhanForm[0] = "ID cau hoi: (tu dong)";      data.loaiTruong[0] = TRUONG_SO;
    data.anNhanKhoa[0] = true;
    data.nhanForm[1] = "Noi dung:";                  data.loaiTruong[1] = TRUONG_NOIDUNG;
    data.nhanForm[2] = "Dap an A:";                  data.loaiTruong[2] = TRUONG_NOIDUNG;
    data.nhanForm[3] = "Dap an B:";                  data.loaiTruong[3] = TRUONG_NOIDUNG;
    data.nhanForm[4] = "Dap an C:";                  data.loaiTruong[4] = TRUONG_NOIDUNG;
    data.nhanForm[5] = "Dap an D:";                  data.loaiTruong[5] = TRUONG_NOIDUNG;
    data.nhanForm[6] = "Dap an dung (A/B/C/D):";     data.loaiTruong[6] = TRUONG_DAPAN;
    data.nhanForm[7] = "  XAC NHAN  ";               data.loaiTruong[7] = TRUONG_NUT;
    const int soDong[8] = { 1, 2,  1,  1,  1,  1,  1,  1 };
    for (int i = 0; i < 8; i++) data.soDongTruong[i] = soDong[i];
    KhoiTaoTrangThai(data.tt);

    NghiepVuCauHoi nv(dsMonHoc, dsLop, maMH);
    data.nghiepVu = &nv;

    ChayManHinhNhap(data);
    return nv.DaDongBoFile();
}