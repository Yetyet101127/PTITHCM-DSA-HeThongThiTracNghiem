#include <sstream>
#include "mylib.h"
#include "Diem.h"
#include "SinhVien.h"
#include "CauHoi.h"
#include "MonHoc.h"
#include "Thi.h"
#include "UI.h"
using namespace std;

string DinhDangDiem(float diem) {
    int v = (int)(diem * 100 + 0.5f);
    int phan = v % 100;
    return SoSangChuoi(v / 100) + "." + (phan < 10 ? "0" : "") + SoSangChuoi(phan);
}

PTRDIEMTHI TimDiemThi(PTRSINHVIEN sv, const string &maMH) {
    for (PTRDIEMTHI d = sv->data.Diem; d != nullptr; d = d->next)
        if (d->data.MAMH == maMH) return d;
    return nullptr;
}

void GiaiPhongDsDiem(PTRDIEMTHI &ds) {
    for (PTRDIEMTHI d = ds; d != nullptr; d = d->next) GiaiPhongDs(d->data.dsCTBT);
    GiaiPhongDs(ds);
}

void NapChiTiet(PTRCTBT &dau, const string &s) {
    PTRCTBT *duoi = &dau;
    size_t i = 0;
    while (i < s.length()) {
        size_t cham = s.find(';', i);
        if (cham == string::npos) cham = s.length();
        size_t hai = s.find(':', i);

        PTRCTBT p = new NodeCHT_SV();
        p->data.id = atoi(s.c_str() + i);
        for (int k = 0; k < 4; k++) p->data.tronDapAn[k] = s[hai + 1 + k] - '0';
        p->data.chonluaSV = (s[hai + 6] == '-') ? ' ' : s[hai + 6];
        *duoi = p;
        duoi = &p->next;
        i = cham + 1;
    }
}

string ChiTietSangChuoi(PTRCTBT dau) {
    string s;
    for (PTRCTBT c = dau; c != nullptr; c = c->next) {
        char ch = c->data.chonluaSV;
        s += SoSangChuoi(c->data.id);
        s += ':';
        for (int k = 0; k < 4; k++) s += (char)('0' + c->data.tronDapAn[k]);
        s += ':';
        s += (ch >= 'A' && ch <= 'D') ? ch : '-';
        if (c->next != nullptr) s += ';';
    }
    return s;
}

// MALOP|MASV|MAMH|DIEM|CHITIET
bool DocFileDiem(DsLOP &dsLop, const string &tenFile) {
    return DocFileTheoDong(tenFile, [&dsLop](string t[], int n) {
        if (n < 5) return;
        PTRSINHVIEN sv = TimSinhVien(dsLop, VietHoa(TrimChuoi(t[1])));
        string maMH = VietHoa(TrimChuoi(t[2]));
        if (sv == nullptr || maMH.empty() || TimDiemThi(sv, maMH) != nullptr) return;

        PTRDIEMTHI d = new NodeDIEMTHI();
        d->data.MAMH = maMH;
        d->data.Diem = (float)atof(t[3].c_str());
        NapChiTiet(d->data.dsCTBT, t[4]);
        ThemCuoi(sv->data.Diem, d);
    });
}

bool GhiFileDiem(const DsLOP &dsLop, const string &tenFile) {
    ofstream f(tenFile.c_str());
    if (!f.is_open()) return false;
    for (int i = 0; i < dsLop.n; i++) {
        for (PTRSINHVIEN sv = dsLop.nodes[i]->Dssv; sv != nullptr; sv = sv->next) {
            for (PTRDIEMTHI d = sv->data.Diem; d != nullptr; d = d->next) {
                f << dsLop.nodes[i]->MALOP << "|" << sv->data.MASV << "|" << d->data.MAMH << "|"
                  << DinhDangDiem(d->data.Diem) << "|" << ChiTietSangChuoi(d->data.dsCTBT) << "\n";
            }
        }
    }
    return true;
}

// ---------- PHIEN THI DANG DO (dangthi.txt) ----------
static string DongPhien(const PhienThi &p) {
    ostringstream o;
    o << p.maLop << "|" << p.maSV << "|" << p.maMH << "|" << p.soPhut << "|" << p.ketThuc << "|"
      << ChiTietSangChuoi(p.dsCTBT);
    return o.str();
}

// cac dong cua sinh vien khac (giu nguyen), bo dong cua maSV
static string GiuDongKhac(const string &maSV) {
    string giu;
    ifstream f(FILE_DANGTHI.c_str());
    if (!f.is_open()) return giu;
    string dong, t[2];
    while (getline(f, dong)) {
        dong = TrimChuoi(dong);
        if (dong.empty()) continue;
        TachChuoi(dong, '|', t, 2);
        if (VietHoa(TrimChuoi(t[1])) != maSV) giu += dong + "\n";
    }
    return giu;
}

bool GhiPhienThi(const PhienThi &p) {
    string noiDung = GiuDongKhac(p.maSV) + DongPhien(p) + "\n";
    ofstream f(FILE_DANGTHI.c_str());
    if (!f.is_open()) return false;
    f << noiDung;
    f.flush();
    return f.good();
}

void XoaPhienThi(const string &maSV) {
    string noiDung = GiuDongKhac(maSV);
    ofstream f(FILE_DANGTHI.c_str());
    if (!f.is_open()) return;
    f << noiDung;
}

// doc het file truoc roi moi goi xuLy, de xuLy duoc phep Ghi/Xoa phien thi
void DuyetPhienThi(std::function<void(PhienThi &)> xuLy) {
    string tat, dong;
    {
        ifstream f(FILE_DANGTHI.c_str());
        if (!f.is_open()) return;
        while (getline(f, dong)) {
            dong = TrimChuoi(dong);
            if (!dong.empty()) tat += dong + "\n";
        }
    }
    size_t i = 0;
    while (i < tat.length()) {
        size_t hetDong = tat.find('\n', i);
        string t[TOI_DA_TRUONG_FILE];
        int n = TachChuoi(tat.substr(i, hetDong - i), '|', t, TOI_DA_TRUONG_FILE);
        i = hetDong + 1;
        if (n < 6) continue;

        PhienThi p;
        p.maLop = VietHoa(TrimChuoi(t[0]));
        p.maSV  = VietHoa(TrimChuoi(t[1]));
        p.maMH  = VietHoa(TrimChuoi(t[2]));
        p.soPhut = atoi(t[3].c_str());
        istringstream(t[4]) >> p.ketThuc;
        NapChiTiet(p.dsCTBT, t[5]);
        xuLy(p);
        GiaiPhongDs(p.dsCTBT);
    }
}

bool DocPhienThi(const string &maSV, PhienThi &kq) {
    bool thay = false;
    DuyetPhienThi([&](PhienThi &p) {
        if (thay || p.maSV != maSV) return;
        kq = p;
        p.dsCTBT = nullptr;   // kq giu danh sach, DuyetPhienThi khong giai phong
        thay = true;
    });
    return thay;
}
bool SinhVienDangThi(const string &maSV) {
    bool co = false;
    DuyetPhienThi([&](PhienThi &p) { if (p.maSV == maSV) co = true; });
    return co;
}
// bang tra cau hoi theo id
static void NapBangId(treeMH n, PTRCAUHOI bang[]) {
    if (n == nullptr) return;
    NapBangId(n->left, bang);
    for (PTRCAUHOI p = n->data.dsCHT; p != nullptr; p = p->next) bang[p->data.id] = p;
    NapBangId(n->right, bang);
}

// moi id trong chi tiet bai thi -> danh dau cau hoi do da thi (khoa sua/xoa)
void DanhDauCauHoiDaThi(const DsLOP &dsLop, treeMH goc) {
    PTRCAUHOI *bang = new PTRCAUHOI[XemIdCauHoiMoi()]();
    NapBangId(goc, bang);

    for (int i = 0; i < dsLop.n; i++) {
        for (PTRSINHVIEN sv = dsLop.nodes[i]->Dssv; sv != nullptr; sv = sv->next)
            for (PTRDIEMTHI d = sv->data.Diem; d != nullptr; d = d->next)
                for (PTRCTBT c = d->data.dsCTBT; c != nullptr; c = c->next) {
                    int id = c->data.id;
                    if (id >= 0 && id < XemIdCauHoiMoi() && bang[id] != nullptr) bang[id]->data.DaThi = true;
                }
    }

    // cau hoi nam trong bai thi dang do cung bi khoa
    DuyetPhienThi([&](PhienThi &p) {
        for (PTRCTBT c = p.dsCTBT; c != nullptr; c = c->next) {
            int id = c->data.id;
            if (id >= 0 && id < XemIdCauHoiMoi() && bang[id] != nullptr) bang[id]->data.DaThi = true;
        }
    });
    delete[] bang;
}


static const string PD_BANG_DIEM = "In bang diem: Danh sach mon  -  Danh sach lop";
static const string PD_CHI_TIET  = "In chi tiet bai thi: Danh sach mon hoc - Danh sach sinh vien da thi";

static const string HD_HAI_BANG =
    "Enter: Danh dau/Bo danh dau   F2: In bang diem\n"
    "Len/Xuong: Chuyen dong   Trai/Phai: Chuyen trang\n"
    "Space: Chuyen bang   Esc: Thoat";
static const string HD_BANG_DIEM =
    "Len/Xuong: Chuyen dong   Trai/Phai: Chuyen trang   Esc: Thoat";
static const string HD_CHI_TIET_BAI_THI =
    "Enter: Danh dau/Bo danh dau   F2: In chi tiet bai thi\n"
    "Len/Xuong: Chuyen dong   Trai/Phai: Chuyen trang\n"
    "Space: Chuyen bang   Esc: Thoat";

static const string LOI_THIEU_DANH_DAU    = "Vui long danh dau du Mon hoc va Lop!";
static const string LOI_CHUA_CO_SV_THI    = "Chua co sinh vien nao thi mon hoc nay!";
static const string LOI_CHUA_DANH_DAU_MON = "Vui long danh dau mon hoc!";
static const string LOI_CHUA_DANH_DAU_SV  = "Vui long danh dau sinh vien!";

static const int PHIM_SPACE = 32;

// bo cuc 2 bang song song
static const int COT_TRAI       = 3;
static const int RONG_VUNG_TRAI = 72;   // rong vung ben trai (bang + huong dan + dong loi)
static const int COT_PHAI       = COT_TRAI + RONG_VUNG_TRAI + 2;
static const int RONG_VUNG_PHAI = 62;

static const int RONG_COT_STT       = 5;
static const int RONG_COT_MA_MH     = 14;
static const int RONG_COT_TEN_MH    = 37;
static const int RONG_COT_SO_CAU_HOI = 12;
static const int RONG_COT_MA_LOP    = 14;
static const int RONG_COT_TEN_LOP   = 29;
static const int RONG_COT_SO_SV     = 10;

static int HangLoi() {
    int h = HANG_BANG + 3 + DONG_MOI_TRANG + 2 + TOI_DA_HUONGDAN + 1;
    return h < CAO_CONSOLE - 1 ? h : CAO_CONSOLE - 2;
}
static void HienLoi(const string &s) { VeDong(HangLoi(), COT_TRAI, s, RONG_VUNG_TRAI, 0, 12); }
static void XoaLoi()                 { VeDong(HangLoi(), COT_TRAI, "", RONG_VUNG_TRAI, 0, 15); }

static void XoaVungPhai() {
    int dau = HANG_BANG - 4;   
    XoaVung(dau, COT_PHAI, RONG_VUNG_PHAI + 1, HangLoi() - dau + 1);
}

class MonHocView : public NghiepVuChiXem {
public:
    MangMonHoc m;   // mon hoc theo ma tang dan

    explicit MonHocView(treeMH goc) { DongBoMangMonHoc(m, goc); }
    ~MonHocView() { GiaiPhongMangMonHoc(m); }

    int LayTongSo() override { return m.soLuong; }
    void LayDuLieuBang(int vt, string cot[], TrangThaiDong &trangThai) override {
        cot[0] = SoSangChuoi(vt + 1);
        cot[1] = m.ds[vt]->data.MAMH;
        cot[2] = m.ds[vt]->data.TENMH;
        cot[3] = SoSangChuoi(DemNut(m.ds[vt]->data.dsCHT));
        trangThai = DONG_BINH_THUONG;
    }
};

// BANG LOP
class LopView : public NghiepVuChiXem {
    const DsLOP &dsLop;
public:
    explicit LopView(const DsLOP &ds) : dsLop(ds) {}

    int LayTongSo() override { return dsLop.n; }
    void LayDuLieuBang(int vt, string cot[], TrangThaiDong &trangThai) override {
        const LOP *l = dsLop.nodes[vt];
        cot[0] = SoSangChuoi(vt + 1);
        cot[1] = l->MALOP;
        cot[2] = l->TENLOP;
        cot[3] = SoSangChuoi(DemNut(l->Dssv));
        trangThai = DONG_BINH_THUONG;
    }
};

static int DuyetSvDaThi(const DsLOP &dsLop, const string &maMH, PTRSINHVIEN *out) {
    int n = 0;
    for (int i = 0; i < dsLop.n; i++) {
        for (PTRSINHVIEN sv = dsLop.nodes[i]->Dssv; sv != nullptr; sv = sv->next)
            if (TimDiemThi(sv, maMH) != nullptr) {
                if (out != nullptr) out[n] = sv;
                n++;
            }
    }
    return n;
}

// BANG SINH VIEN
class SvView : public NghiepVuChiXem {
public:
    PTRSINHVIEN *ds;
    int n;
    string maMH;
    bool coDiem;   

    // SV cua 1 lop, kem diem mon ma
    SvView(LOP *lop, const string &ma) : maMH(ma), coDiem(true) {
        n = DemNut(lop->Dssv);
        ds = new PTRSINHVIEN[n];
        int i = 0;
        for (PTRSINHVIEN p = lop->Dssv; p != nullptr; p = p->next) ds[i++] = p;
    }
    // SV da thi mon ma, khong co cot DIEM
    SvView(const DsLOP &dsLop, const string &ma) : maMH(ma), coDiem(false) {
        n = DuyetSvDaThi(dsLop, ma, nullptr);
        ds = new PTRSINHVIEN[n];
        DuyetSvDaThi(dsLop, ma, ds);
    }
    ~SvView() { delete[] ds; }

    int LayTongSo() override { return n; }
    void LayDuLieuTimKiem(int vt, string cot[]) override {
        cot[2] = ds[vt]->data.HO + " " + ds[vt]->data.TEN;
    }
    void LayDuLieuBang(int vt, string cot[], TrangThaiDong &trangThai) override {
        const SINHVIEN &sv = ds[vt]->data;
        cot[0] = SoSangChuoi(vt + 1);
        cot[1] = sv.MASV;
        cot[2] = sv.HO;
        cot[3] = sv.TEN;
        cot[4] = sv.PHAI;
        if (coDiem) {
            PTRDIEMTHI d = TimDiemThi(ds[vt], maMH);
            cot[5] = (d != nullptr) ? DinhDangDiem(d->data.Diem) : "Chua thi";
        }
        trangThai = DONG_BINH_THUONG;
    }
};

static void CauHinhChung(DuLieuManHinhNhap &d, PTRNGHIEPVU nv, const string &tenTim) {
    d.soTruongForm = 0;
    KhoiTaoTrangThai(d.tt);
    d.nghiepVu = nv;
    d.cheDoChiChon = true;
    d.coTimKiem = true;
    d.cotTimKiem = 2;
    d.tenTimKiem = tenTim;
    d.loaiTimKiem = TRUONG_CHU;
}

static void CauHinhBangChon(DuLieuManHinhNhap &d, PTRNGHIEPVU nv, int col, const string &donVi,
                            const string &cotMa, int rongMa, const string &cotTen, int rongTen,
                            const string &cotSo, int rongSo, const string &tenTim) {
    KhoiTaoBang(d.bang, HANG_BANG, col, donVi);
    ThemCot(d.bang, "STT", RONG_COT_STT);
    ThemCot(d.bang, cotMa, rongMa);
    ThemCot(d.bang, cotTen, rongTen);
    ThemCot(d.bang, cotSo, rongSo);
    CauHinhChung(d, nv, tenTim);
}

static void CauHinhBangMon(DuLieuManHinhNhap &d, PTRNGHIEPVU nv) {
    CauHinhBangChon(d, nv, COT_TRAI, "mon hoc", "MA MH", RONG_COT_MA_MH, "TEN MON HOC", RONG_COT_TEN_MH,
                    "SO CAU HOI", RONG_COT_SO_CAU_HOI, "ten mon hoc");
}

static void CauHinhBangLop(DuLieuManHinhNhap &d, PTRNGHIEPVU nv) {
    CauHinhBangChon(d, nv, COT_PHAI, "lop", "MA LOP", RONG_COT_MA_LOP, "TEN LOP", RONG_COT_TEN_LOP,
                    "SO SV", RONG_COT_SO_SV, "ten lop");
}

static void CauHinhBangSV(DuLieuManHinhNhap &d, PTRNGHIEPVU nv, int col, bool coDiem) {
    KhoiTaoBang(d.bang, HANG_BANG, col, "sinh vien");
    ThemCot(d.bang, "STT", 5);
    ThemCot(d.bang, "MA SV", 13);
    ThemCot(d.bang, "HO VA TEN DEM", 22);
    ThemCot(d.bang, "TEN", 12);
    ThemCot(d.bang, "PHAI", 6);
    if (coDiem) ThemCot(d.bang, "DIEM", 10);
    CauHinhChung(d, nv, "ho ten");
}


template<class F>
static bool XuLyPhimBang(BangLoc &bl, int phim, F ve) {
    if (bl.dangTim) {
        if (XuLyPhimTimBangLoc(bl, phim)) ve();
        return true;
    }
    if (phim == PHIM_F4) {
        if (BatDauTimBangLoc(bl)) ve();
        return true;
    }
    if (LaPhimMuiTen(phim)) {
        if (DiChuyenBangLoc(bl, phim)) VeCacDongBangLoc(bl, true);
        return true;
    }
    return false;
}

static void DanhDauMotDong(BangLoc &bl) {
    int idx = ChiSoDongChon(bl);
    if (idx < 0) return;
    bool daDanhDau = CoDanhDauBangLoc(bl, idx);
    XoaDanhDauBangLoc(bl);
    if (!daDanhDau) DoiDanhDauBangLoc(bl, idx);
    VeCacDongBangLoc(bl, true);
}

static void InBangDiemLop(treeMH mh, LOP *lop) {
    SvView bd(lop, mh->data.MAMH);
    DuLieuManHinhNhap d;
    d.tieuDe = "In bang diem: Mon " + mh->data.MAMH + " - Lop " + lop->MALOP;
    CauHinhBangSV(d, &bd, COT_TRAI, true);
    d.huongDanXem = HD_BANG_DIEM;
    ChayManHinhNhap(d);
}

static bool LaPhimHopLe2Bang(int phim) {
    return phim == PHIM_ENTER || phim == PHIM_F2 || phim == PHIM_F4 || phim == PHIM_ESC || phim == PHIM_SPACE ||
           LaPhimMuiTen(phim);
}

struct HaiBangMonLop {
    DsLOP &dsLop;
    MonHocView mon;
    LopView lop;
    DuLieuManHinhNhap dMon, dLop;
    BangLoc bangMon, bangLop;
    int focus;  
    int dongMoiTrangCu;

    HaiBangMonLop(treeMH goc, DsLOP &ds) : dsLop(ds), mon(goc), lop(ds), focus(0), dongMoiTrangCu(DONG_MOI_TRANG) {
        CauHinhBangMon(dMon, &mon);
        CauHinhBangLop(dLop, &lop);
        DONG_MOI_TRANG = dMon.dongMoiTrang;   // dung chung cho 2 bang
        KhoiTaoBangLoc(bangMon, dMon, RONG_VUNG_TRAI);
        KhoiTaoBangLoc(bangLop, dLop, RONG_VUNG_PHAI);
    }
    ~HaiBangMonLop() {
        GiaiPhongBangLoc(bangMon);
        GiaiPhongBangLoc(bangLop);
        DONG_MOI_TRANG = dongMoiTrangCu;
    }

    BangLoc &HienHanh() { return focus == 0 ? bangMon : bangLop; }

    void Ve() {
        VeBangLoc(bangMon, focus == 0, HD_HAI_BANG);
        VeBangLoc(bangLop, focus == 1, "");
    }

    void VaoManHinh() {
        VeNenManHinh(PD_BANG_DIEM);
        Ve();
    }

    bool Chon(treeMH &monChon, LOP *&lopChon) {
        bool coLoi = false;
        while (true) {
            int phim = DocPhim();
            if (!HienHanh().dangTim && !LaPhimHopLe2Bang(phim)) continue;   
            if (coLoi) { XoaLoi(); coLoi = false; }

            if (XuLyPhimBang(HienHanh(), phim, [&]() { Ve(); })) continue;

            if (phim == PHIM_ESC) return false;
            if (phim == PHIM_SPACE) {
                focus = 1 - focus;
                Ve();
            }
            else if (phim == PHIM_ENTER) {
                DanhDauMotDong(HienHanh());
            }
            else if (phim == PHIM_F2) {
                int im[1], il[1];
                bool du = LayDanhDauBangLoc(bangMon, im, 1) == 1 && LayDanhDauBangLoc(bangLop, il, 1) == 1;
                if (du) {
                    monChon = mon.m.ds[im[0]];
                    lopChon = dsLop.nodes[il[0]];
                    return true;
                }
                HienLoi(LOI_THIEU_DANH_DAU);
                coLoi = true;
            }
        }
    }
};

void InBangDiem(treeMH goc, DsLOP &dsLop) {
    HaiBangMonLop hb(goc, dsLop);
    treeMH monChon = nullptr;
    LOP *lopChon = nullptr;
    while (true) {
        hb.VaoManHinh();
        if (!hb.Chon(monChon, lopChon)) break;
        InBangDiemLop(monChon, lopChon);
    }
}


class MonHocCoCauHoiView : public NghiepVuChiXem {
public:
    treeMH *ds;
    int n;

    explicit MonHocCoCauHoiView(treeMH goc) {
        MangMonHoc m;
        DongBoMangMonHoc(m, goc);
        ds = new treeMH[m.soLuong];
        n = 0;
        for (int i = 0; i < m.soLuong; i++)
            if (m.ds[i]->data.dsCHT != nullptr) ds[n++] = m.ds[i];
        GiaiPhongMangMonHoc(m);
    }
    ~MonHocCoCauHoiView() { delete[] ds; }

    int LayTongSo() override { return n; }
    void LayDuLieuBang(int vt, string cot[], TrangThaiDong &trangThai) override {
        trangThai = DONG_BINH_THUONG;
        if (vt < 0 || vt >= n) return;
        cot[0] = SoSangChuoi(vt + 1);
        cot[1] = ds[vt]->data.MAMH;
        cot[2] = ds[vt]->data.TENMH;
        cot[3] = SoSangChuoi(DemNut(ds[vt]->data.dsCHT));
    }
};

struct BangPhaiSV {
    SvView *sv = nullptr;
    DuLieuManHinhNhap d;
    BangLoc bl;
    bool dangMo = false;

    void Dong() {
        if (!dangMo) return;
        GiaiPhongBangLoc(bl);
        delete sv;
        sv = nullptr;
        dangMo = false;
    }
    void Mo(SvView *moi) {
        Dong();
        sv = moi;
        CauHinhBangSV(d, sv, COT_PHAI, false);
        bl = BangLoc();
        KhoiTaoBangLoc(bl, d, RONG_VUNG_PHAI);
        dangMo = true;
    }
    ~BangPhaiSV() { Dong(); }
};


void InChiTietBaiThi(treeMH goc, DsLOP &dsLop) {
    MonHocCoCauHoiView mon(goc);
    BangPhaiSV phai;

    DuLieuManHinhNhap dMon;
    CauHinhBangMon(dMon, &mon);
    const int dongMoiTrangCu = DONG_MOI_TRANG;
    DONG_MOI_TRANG = dMon.dongMoiTrang;
    BangLoc bangMon;
    KhoiTaoBangLoc(bangMon, dMon, RONG_VUNG_TRAI);

    int focus = 0;   // 0: bang mon, 1: bang SV
    treeMH monChon = nullptr;

    auto ve = [&]() {
        VeBangLoc(bangMon, focus == 0, HD_CHI_TIET_BAI_THI);
        if (phai.dangMo) VeBangLoc(phai.bl, focus == 1, "");
    };
    auto veToanMan = [&]() {   
        VeNenManHinh(PD_CHI_TIET);
        ve();
    };
    auto dongBangPhai = [&]() {
        bool dangMoTruoc = phai.dangMo;
        phai.Dong();
        monChon = nullptr;
        focus = 0;
        if (dangMoTruoc) XoaVungPhai();
        ve();
    };

    veToanMan();

    bool coLoi = false;
    while (true) {
        int phim = DocPhim();
        BangLoc &bl = (focus == 1 && phai.dangMo) ? phai.bl : bangMon;
        if (!bl.dangTim && !LaPhimHopLe2Bang(phim)) continue;   
        if (coLoi) { XoaLoi(); coLoi = false; }

        if (XuLyPhimBang(bl, phim, ve)) continue;

        if (phim == PHIM_ESC) break;

        if (phim == PHIM_SPACE) {
            if (phai.dangMo) { focus = 1 - focus; ve(); }
            continue;
        }
        
        if (phim == PHIM_F2) {
            if (!phai.dangMo) {
                HienLoi(LOI_CHUA_DANH_DAU_MON);
                coLoi = true;
                continue;
            }
            int i;
            if (LayDanhDauBangLoc(phai.bl, &i, 1) == 0) {
                HienLoi(LOI_CHUA_DANH_DAU_SV);
                coLoi = true;
                continue;
            }
            XemChiTietBaiThi(goc, phai.sv->ds[i], monChon);
            veToanMan();
            continue;
        }

        if (phim == PHIM_ENTER && focus == 1) {
            DanhDauMotDong(phai.bl);
            continue;
        }

        if (phim == PHIM_ENTER && focus == 0) {
            int idx = ChiSoDongChon(bangMon);
            if (idx < 0) continue;
            bool daDanhDau = CoDanhDauBangLoc(bangMon, idx);
            XoaDanhDauBangLoc(bangMon);

            if (daDanhDau) {
                dongBangPhai();
                continue;
            }

            DoiDanhDauBangLoc(bangMon, idx);
            treeMH mh = mon.ds[idx];
            SvView *moi = new SvView(dsLop, mh->data.MAMH);
            if (moi->n == 0) {
                delete moi;
                dongBangPhai();
                HienLoi(LOI_CHUA_CO_SV_THI);
                coLoi = true;
            } else {
                phai.Mo(moi);
                monChon = mh;
                ve();
            }
        }
    }

    GiaiPhongBangLoc(bangMon);
    XoaThanManHinh();
    DONG_MOI_TRANG = dongMoiTrangCu;
}