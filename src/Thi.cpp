#include "mylib.h"
#include "Thi.h"
#include "MonHoc.h"
#include "Lop.h"
#include "Diem.h"
#include "UI.h"
#include <ctime>
using namespace std;

static string HaiSo(int x) {
    return string(x < 10 ? "0" : "") + SoSangChuoi(x);
}

static int SoChuSo(int x) {
    int n = 1;
    while (x >= 10) { x /= 10; n++; }
    return n;
}

// BO CUC MAN HINH
static const int COT_TRAI         = 3;
static const int RONG_TRAI        = 96;
static const int RONG_CHU         = RONG_TRAI - 4;   // chu trong khung dap an
static const int COT_NGAN         = COT_TRAI + RONG_TRAI + 1;   // vach doc ngan 2 ben
static const int COT_PHAI         = COT_NGAN + 2;
static const int RONG_PHAI        = RONG_CONSOLE - COT_PHAI - 2;

static const int HANG_MASV        = 5;
static const int HANG_HOTEN       = 6;
static const int HANG_NGAN        = 8;   // vach ngang
static const int HANG_NOIDUNG     = HANG_NGAN + 1;   // noi dung cau hoi (toi da 4 dong)
static const int HANG_DAPAN       = HANG_NOIDUNG + 5;   // 4 khung dap an
static const int SO_DONG_DAP_AN   = 4;   // so dong chu trong 1 khung
static const int CAO_KHUNG_DAP_AN = SO_DONG_DAP_AN + 2;   // + 2 vien
static const int HANG_HUONGDAN    = HANG_NGAN + 1;   // huong dan ben phai
static const int TOI_DA_HUONGDAN_THI = 3;
static const int SO_CAU_MOI_DONG  = 5;
// luoi cau hoi: sau huong dan den het man hinh
static const int SO_DONG_LUOI     = (CAO_CONSOLE - 2) - (HANG_HUONGDAN + TOI_DA_HUONGDAN_THI + 2) + 1;
static const int O_MOI_NHOM       = SO_CAU_MOI_DONG * SO_DONG_LUOI;

struct BaiThi {
    PTRSINHVIEN sv;
    treeMH mh;
    int soCau;
    PTRCAUHOI *de;
    char *chon;
    int daTraLoi;
    bool cheDoXem;   // true: xem lai bai da nop
    int dung;   // chi o che do xem
    float diem;   // chi o che do xem
};

static int SoHuongDan(const BaiThi &bt) { return bt.cheDoXem ? 2 : TOI_DA_HUONGDAN_THI; }
static int HangLuoi(const BaiThi &bt)   { return HANG_HUONGDAN + SoHuongDan(bt) + 1; }
static int HangO(const BaiThi &bt)      { return HangLuoi(bt) + 1; }

static void VeHuongDanBenPhai(const BaiThi &bt) {
    string hd[TOI_DA_HUONGDAN_THI];
    if (bt.cheDoXem) {
        hd[0] = "Len/Xuong/Trai/Phai: Chuyen cau hoi";
        hd[1] = "Esc: Thoat";
    } else {
        hd[0] = "A/B/C/D: Chon dap an";
        hd[1] = "Len/Xuong/Trai/Phai: Chuyen cau hoi";
        hd[2] = "Esc: Nop bai";
    }
    for (int i = 0; i < SoHuongDan(bt); i++) VeDongHuongDan(HANG_HUONGDAN + i, COT_PHAI, hd[i], RONG_PHAI);
}

static void VeKetQuaXem(const BaiThi &bt) {
    string tong = SoSangChuoi(bt.soCau);
    int boTrong = bt.soCau - bt.daTraLoi;
    int sai = bt.daTraLoi - bt.dung;
    VeDong(HANG_MASV, COT_PHAI,
           "Diem: " + DinhDangDiem(bt.diem) + " (" + SoSangChuoi(bt.dung) + "/" + tong + ")", RONG_PHAI, 0, 11);
    VeDong(HANG_HOTEN, COT_PHAI,
           "Sai: " + SoSangChuoi(sai) + "/" + tong + "   Bo trong: " + SoSangChuoi(boTrong) + "/" + tong,
           RONG_PHAI, 0, 15);
}

static void VeDaTraLoi(const BaiThi &bt) {
    VeDong(HANG_HOTEN, COT_PHAI, "Da tra loi: " + SoSangChuoi(bt.daTraLoi) + "/" + SoSangChuoi(bt.soCau),
           RONG_PHAI, 0, 10);
}

static void VeDongNhan(int row, int col, const string &s, int rong, int nen, int chuGiaTri);

static void VeKhung(const BaiThi &bt) {
    string phuDe = string(bt.cheDoXem ? "Xem chi tiet bai thi" : "Thi trac nghiem") + " - Mon: " + bt.mh->data.MAMH;
    VeNenManHinh(phuDe);
    VeDongNhan(HANG_MASV,  COT_TRAI, RutGon("Ma sinh vien: " + bt.sv->data.MASV, 60), 60, 0, 15);
    VeDongNhan(HANG_HOTEN, COT_TRAI, RutGon("Ho va ten: " + bt.sv->data.HO + " " + bt.sv->data.TEN, 60), 60, 0, 15);

    int rongNgan = RONG_CONSOLE - 2 * COT_TRAI;
    VeDong(HANG_NGAN, COT_TRAI, string(rongNgan, '-'), rongNgan, 0, 7);
    for (int r = HANG_NGAN + 1; r < CAO_CONSOLE - 1; r++) VeDong(r, COT_NGAN, "|", 1, 0, 7);

    VeHuongDanBenPhai(bt);
    if (bt.cheDoXem) VeKetQuaXem(bt);
    else             VeDaTraLoi(bt);
}

static void VeDongHo(int giay) {
    string s = "Thoi gian con lai: " + HaiSo(giay / 60) + " phut " + HaiSo(giay % 60) + " giay";
    VeDong(HANG_MASV, COT_PHAI, s, RONG_PHAI, 0, giay <= 60 ? 12 : 14);
}

static void VeNoiDung(const BaiThi &bt, int cur) {
    string dong[4];
    int n = ChiaDongVanBan("Cau " + SoSangChuoi(cur + 1) + ": " + bt.de[cur]->data.NoiDung, RONG_CHU, dong, 4);
    for (int i = 0; i < 4; i++)
        VeDong(HANG_NOIDUNG + i, COT_TRAI, i < n ? dong[i] : "", RONG_TRAI, 0, 14);
}

static int MauVienDapAn(const BaiThi &bt, int cur, char kt) {
    const CAUHOI &ch = bt.de[cur]->data;
    if (bt.cheDoXem) {
        if (kt == ch.DapAn)          return 10;
        if (kt == bt.chon[cur])      return 12;
    } else if (kt == bt.chon[cur])   return 10;
    return 15;
}

static void VeDapAn(const BaiThi &bt, int cur, int k, bool veChu = true) {
    const CAUHOI &ch = bt.de[cur]->data;
    char kt = (char)('A' + k);
    int mauVien = MauVienDapAn(bt, cur, kt);
    int hang = HANG_DAPAN + CAO_KHUNG_DAP_AN * k;
    int cotPhaiKhung = COT_TRAI + RONG_TRAI - 1;
    string vien(RONG_TRAI, '-');

    string dong[SO_DONG_DAP_AN];
    int n = 0;
    if (veChu) {
        const string &nd = (k == 0) ? ch.a : (k == 1) ? ch.b : (k == 2) ? ch.c : ch.d;
        n = ChiaDongVanBan(string(1, kt) + ". " + nd, RONG_CHU, dong, SO_DONG_DAP_AN);
    }

    VeDong(hang, COT_TRAI, vien, RONG_TRAI, 0, mauVien);
    for (int i = 0; i < SO_DONG_DAP_AN; i++) {
        int r = hang + 1 + i;
        VeDong(r, COT_TRAI, "|", 1, 0, mauVien);
        if (veChu) VeDong(r, COT_TRAI + 1, " " + (i < n ? dong[i] : string("")), RONG_TRAI - 2, 0, 15);
        VeDong(r, cotPhaiKhung, "|", 1, 0, mauVien);
    }
    VeDong(hang + SO_DONG_DAP_AN + 1, COT_TRAI, vien, RONG_TRAI, 0, mauVien);
}

static int CotO(const BaiThi &bt, int viTri) {
    return COT_PHAI + (viTri % SO_CAU_MOI_DONG) * (SoChuSo(bt.soCau) + 5);
}

static void VeOLuoi(const BaiThi &bt, int cur, int idx) {
    int viTri = idx - (cur / O_MOI_NHOM) * O_MOI_NHOM;
    if (viTri < 0 || viTri >= O_MOI_NHOM) return;

    char c = bt.chon[idx];
    int nen = (idx == cur) ? 8 : 0, chu = 15;
    if (c != ' ') {
        if (bt.cheDoXem) chu = (c == bt.de[idx]->data.DapAn) ? 10 : 12;
        else             chu = 10;
    } else if (bt.cheDoXem) chu = 14;

    VeDong(HangO(bt) + viTri / SO_CAU_MOI_DONG, CotO(bt, viTri),
           SoSangChuoi(idx + 1) + ":" + string(1, c == ' ' ? '-' : c), SoChuSo(bt.soCau) + 2, nen, chu);
}

static void VeLuoi(const BaiThi &bt, int cur) {
    int nhom = cur / O_MOI_NHOM;
    int soNhom = (bt.soCau + O_MOI_NHOM - 1) / O_MOI_NHOM;
    string tieuDe = "Danh sach cau hoi";
    if (soNhom > 1) tieuDe += "   Nhom " + SoSangChuoi(nhom + 1) + "/" + SoSangChuoi(soNhom);
    VeDong(HangLuoi(bt), COT_PHAI, tieuDe, RONG_PHAI, 0, 9);

    for (int v = 0; v < O_MOI_NHOM; v++) {
        int idx = nhom * O_MOI_NHOM + v;
        if (idx < bt.soCau) VeOLuoi(bt, cur, idx);
        else VeDong(HangO(bt) + v / SO_CAU_MOI_DONG, CotO(bt, v), "", SoChuSo(bt.soCau) + 2, 0, 15);
    }
}

static const int HANG_LUA_CHON = HANG_DAPAN + 4 * CAO_KHUNG_DAP_AN + 1;

static void VeLuaChonXem(const BaiThi &bt, int cur) {
    if (!bt.cheDoXem) return;

    char dapAn = bt.de[cur]->data.DapAn;
    char chon  = bt.chon[cur];
    bool daChon = (chon >= 'A' && chon <= 'D');

    int mau = !daChon ? 14 : (chon == dapAn ? 10 : 12);
    string giaTriChon = daChon ? string(1, chon) : "-";
    string ketQua     = !daChon ? "-" : (chon == dapAn ? "DUNG" : "SAI");

    const string n1 = "Sinh vien chon: ";
    const string n2 = "Dap an dung: ";
    const string n3 = "Ket qua: ";

    int col = COT_TRAI;
    VeDong(HANG_LUA_CHON, col, n1, (int)n1.length(), 0, 9);          col += (int)n1.length();
    VeDong(HANG_LUA_CHON, col, giaTriChon, 13, 0, mau);              col += 4;
    VeDong(HANG_LUA_CHON, col, n2, (int)n2.length(), 0, 9);          col += (int)n2.length();
    VeDong(HANG_LUA_CHON, col, string(1, dapAn), 4, 0, 10);          col += 4;
    VeDong(HANG_LUA_CHON, col, n3, (int)n3.length(), 0, 9);          col += (int)n3.length();
    VeDong(HANG_LUA_CHON, col, ketQua, 10, 0, mau);
}

static void VeCauHoi(const BaiThi &bt, int cur) {
    VeNoiDung(bt, cur);
    for (int k = 0; k < 4; k++) VeDapAn(bt, cur, k);
    VeLuaChonXem(bt, cur);
}

static void VeCau(const BaiThi &bt, int cur) {
    VeCauHoi(bt, cur);
    VeLuoi(bt, cur);
}

static void ChuyenCau(const BaiThi &bt, int &cur, int moi) {
    if (moi == cur) return;
    int cu = cur;
    cur = moi;
    if (cu / O_MOI_NHOM != cur / O_MOI_NHOM) { VeCau(bt, cur); return; }
    VeCauHoi(bt, cur);
    VeOLuoi(bt, cur, cu);
    VeOLuoi(bt, cur, cur);
}

// buoc: +-1 trai/phai, +-5 len/xuong 1 dong luoi
static int ChonCau(int cur, int soCau, int buoc) {
    int moi = cur + buoc;
    if (moi < 0) return cur;
    if (moi >= soCau)
        return (buoc > 1 && cur / SO_CAU_MOI_DONG < (soCau - 1) / SO_CAU_MOI_DONG) ? soCau - 1 : cur;
    return moi;
}

static void XoaBoDemPhim() {
    while (kbhit()) getch();
}

struct DongKetQua { string s; int nen, chu; };
static const int RONG_KQ_TOI_THIEU = 64;
static const int TOI_DA_DONG_KQ    = 16;

static void VeDongNhan(int row, int col, const string &s, int rong, int nen, int chuGiaTri) {
    string t = s;
    if ((int)t.length() < rong) t += string(rong - t.length(), ' ');
    if (t.find(':') == string::npos) { VeDong(row, col, t, rong, nen, chuGiaTri); return; }

    int n = (int)t.length(), dau = 0;
    bool laNhan = true;
    for (int i = 0; i <= n; i++) {
        bool doi = (i == n);
        if (!doi && !laNhan && t[i] == ' ' && i + 1 < n && t[i + 1] == ' ') doi = true;   // het gia tri
        else if (!doi && laNhan && i > 0 && t[i - 1] == ':') doi = true;   // het nhan
        if (!doi) continue;
        if (i > dau) VeDong(row, col + dau, t.substr(dau, i - dau), i - dau, nen, laNhan ? 9 : chuGiaTri);
        dau = i;
        laNhan = !laNhan;
    }
}

static void VeKetQua(const BaiThi &bt, int dung, float diem, bool hetGio, bool luuOk, int soPhut) {
    VeNenManHinh("Ket qua Bai thi - Mon: " + bt.mh->data.MAMH);

    string tong = SoSangChuoi(bt.soCau);
    int boTrong = bt.soCau - bt.daTraLoi;
    int sai = bt.daTraLoi - dung;

    DongKetQua dong[TOI_DA_DONG_KQ];
    int n = 0;
    dong[n++] = { "", 15, 0 };
    if (hetGio) dong[n++] = { "  Het thoi gian! He thong tu dong nop bai.", 15, 4 };
    dong[n++] = { "  Ma sinh vien: " + bt.sv->data.MASV, 15, 0 };
    dong[n++] = { "  Ho ten      : " + bt.sv->data.HO + " " + bt.sv->data.TEN, 15, 0 };
    dong[n++] = { "  So cau: " + tong, 15, 0 };
    dong[n++] = { "  Thoi gian lam bai: " + SoSangChuoi(soPhut) + " phut", 15, 0 };
    dong[n++] = { "  Dung: " + SoSangChuoi(dung) + "/" + tong + "   Sai: " + SoSangChuoi(sai) + "/" + tong +
                  "   Bo trong: " + SoSangChuoi(boTrong) + "/" + tong, 15, 0 };
    dong[n++] = { "", 15, 0 };
    dong[n++] = { "  DIEM: " + DinhDangDiem(diem) + " / 10", 11, 0 };
    dong[n++] = { "", 15, 0 };
    if (luuOk) dong[n++] = { "  Da luu ket qua bai thi.", 15, 2 };
    else       dong[n++] = { "  Loi ghi file! Du lieu chua duoc luu.", 15, 4 };
    dong[n++] = { "", 15, 0 };

    const string tieuDe = "KET QUA - MON " + bt.mh->data.MAMH;
    const string nhacPhim = "Nhan phim bat ky de quay lai menu...";
    int rong = RONG_KQ_TOI_THIEU;
    int dai = (int)tieuDe.length();
    for (int i = 0; i < n; i++) if ((int)dong[i].s.length() > dai) dai = (int)dong[i].s.length();
    if (dai + 2 > rong) rong = dai + 2;
    if (rong > RONG_CONSOLE - 2) rong = RONG_CONSOLE - 2;
    const int col = (RONG_CONSOLE - rong) / 2;

    int hang = 9;
    VeDong(hang++, col, CanGiua(RutGon(tieuDe, rong), rong), rong, 14, 12);
    for (int i = 0; i < n; i++) VeDongNhan(hang++, col, RutGon(dong[i].s, rong), rong, dong[i].nen, dong[i].chu);
    VeDong(hang + 1, col, nhacPhim, rong, 0, 7);
    XoaBoDemPhim();
    DocPhim();
}

static PTRCAUHOI* BocDe(treeMH mh, int tong, int soCau) {
    PTRCAUHOI *de = new PTRCAUHOI[tong];
    int k = 0;
    for (PTRCAUHOI p = mh->data.dsCHT; p != nullptr; p = p->next) de[k++] = p;
    srand((unsigned)time(NULL));
    for (int i = 0; i < soCau; i++) {
        int j = i + rand() % (tong - i);
        PTRCAUHOI t = de[i]; de[i] = de[j]; de[j] = t;
    }
    return de;
}

// dem gio cho bai thi: vua dung de kiem tra dieu kien dung vong lap chinh,
// vua truyen cho XacNhanCoThoiGian de dong ho tiep tuc chay trong luc hoi "Nop bai som?"
class DemGioThi : public IKiemTraHetGio {
private:
    DWORD batDau;
    DWORD tongMs;
    int giayHienThi;

public:
    explicit DemGioThi(int soPhut) : batDau(GetTickCount()), tongMs((DWORD)soPhut * 60000UL), giayHienThi(-1) {}

    // tra ve true neu da het gio; neu chua, cap nhat dong ho hien thi khi so giay thay doi
    bool HetGio() override {
        DWORD troi = GetTickCount() - batDau;
        if (troi >= tongMs) return true;
        int giay = (int)((tongMs - troi + 999) / 1000);
        if (giay != giayHienThi) { giayHienThi = giay; VeDongHo(giay); }
        return false;
    }
};

static bool ChayVongThi(BaiThi &bt, int soPhut) {
    DemGioThi dongHo(soPhut);
    int cur = 0;

    AnHienConTro(false);
    VeKhung(bt);
    VeCau(bt, cur);

    while (true) {
        if (!bt.cheDoXem) {
            if (dongHo.HetGio()) return true;
        }

        if (!kbhit()) { Sleep(50); continue; }
        int phim = DocPhim();

        if (phim == PHIM_ESC) {
            if (bt.cheDoXem) return false;
            int chua = bt.soCau - bt.daTraLoi;
            string hoi = (chua > 0) ? "Con " + SoSangChuoi(chua) + " cau chua tra loi. Nop bai som?"
                                    : string("Ban co chac muon nop bai?");

            // dong ho van tiep tuc chay trong luc cho xac nhan; het gio ngay trong luc hoi -> tu dong nop bai
            KetQuaXacNhan kq = XacNhanCoThoiGian(hoi, &dongHo);
            if (kq == XN_HETGIO) return true;
            if (kq == XN_DONGY)  return false;
            // XN_HUY: tiep tuc lam bai, khong lam gi them
        }
        else if (phim == PHIM_TRAI)  ChuyenCau(bt, cur, ChonCau(cur, bt.soCau, -1));
        else if (phim == PHIM_PHAI)  ChuyenCau(bt, cur, ChonCau(cur, bt.soCau, 1));
        else if (phim == PHIM_LEN)   ChuyenCau(bt, cur, ChonCau(cur, bt.soCau, -SO_CAU_MOI_DONG));
        else if (phim == PHIM_XUONG) ChuyenCau(bt, cur, ChonCau(cur, bt.soCau, SO_CAU_MOI_DONG));
        else if (!bt.cheDoXem && ((phim >= 'a' && phim <= 'd') || (phim >= 'A' && phim <= 'D'))) {
            char c = (char)toupper(phim);
            char cu = bt.chon[cur];
            if (cu != c) {
                if (cu == ' ') bt.daTraLoi++;
                bt.chon[cur] = c;
                // chi ve lai vien khung, khong ve lai chu
                if (cu != ' ') VeDapAn(bt, cur, cu - 'A', false);
                VeDapAn(bt, cur, c - 'A', false);
                VeDaTraLoi(bt);
                VeOLuoi(bt, cur, cur);
            }
        }
    }
}

static int DemCauDung(const BaiThi &bt) {
    int dung = 0;
    for (int i = 0; i < bt.soCau; i++)
        if (bt.chon[i] == bt.de[i]->data.DapAn) dung++;
    return dung;
}

static bool LuuKetQuaThi(DsLOP &dsLop, const BaiThi &bt, float diem) {
    PTRDIEMTHI d = new NodeDIEMTHI();
    d->data.MAMH = bt.mh->data.MAMH;
    d->data.Diem = diem;
    PTRCTBT *duoi = &d->data.dsCTBT;
    for (int i = 0; i < bt.soCau; i++) {
        *duoi = new NodeCHT_SV();
        (*duoi)->data.id = bt.de[i]->data.id;
        (*duoi)->data.chonluaSV = bt.chon[i];
        duoi = &((*duoi)->next);
        bt.de[i]->data.DaThi = true;   // khoa sua/xoa cau hoi
    }
    ThemCuoi(bt.sv->data.Diem, d);
    return LuuDuLieu(dsLop);
}

static void LamBaiThi(DsLOP &dsLop, PTRSINHVIEN sv, treeMH mh, int soCau, int soPhut) {
    int tong = DemNut(mh->data.dsCHT);
    if (soCau > tong) soCau = tong;
    if (soCau <= 0) return;

    BaiThi bt;
    bt.sv = sv; bt.mh = mh; bt.soCau = soCau; bt.daTraLoi = 0;
    bt.cheDoXem = false; bt.dung = 0; bt.diem = 0;
    bt.de = BocDe(mh, tong, soCau);
    bt.chon = new char[soCau];
    for (int i = 0; i < soCau; i++) bt.chon[i] = ' ';

    bool hetGio = ChayVongThi(bt, soPhut);
    int dung = DemCauDung(bt);
    float diem = (float)dung * 10.0f / (float)soCau;
    bool luuOk = LuuKetQuaThi(dsLop, bt, diem);

    VeKetQua(bt, dung, diem, hetGio, luuOk, soPhut);

    delete[] bt.chon;
    delete[] bt.de;
}

static PTRCAUHOI TimCauTheoId(PTRCAUHOI ds, int id) {
    for (PTRCAUHOI p = ds; p != nullptr; p = p->next)
        if (p->data.id == id) return p;
    return nullptr;
}

// dung lai bai thi tu dsCTBT, mo che do xem
void XemChiTietBaiThi(treeMH gocMH, PTRSINHVIEN sv, treeMH mh) {
    if (sv == nullptr || mh == nullptr) return;
    mh = TimMonHoc(gocMH, mh->data.MAMH);
    PTRDIEMTHI d = (mh != nullptr) ? TimDiemThi(sv, mh->data.MAMH) : nullptr;
    if (d == nullptr) return;

    int tongCT = DemNut(d->data.dsCTBT);
    BaiThi bt;
    bt.sv = sv; bt.mh = mh; bt.soCau = tongCT; bt.daTraLoi = 0;
    bt.cheDoXem = true;
    bt.de = new PTRCAUHOI[tongCT];
    bt.chon = new char[tongCT];
    int i = 0;
    for (PTRCTBT c = d->data.dsCTBT; c != nullptr; c = c->next, i++) {
        bt.de[i] = TimCauTheoId(mh->data.dsCHT, c->data.id);
        bt.chon[i] = c->data.chonluaSV;
        if (c->data.chonluaSV != ' ') bt.daTraLoi++;
    }

    bt.dung = DemCauDung(bt);
    bt.diem = d->data.Diem;
    ChayVongThi(bt, 0);

    delete[] bt.chon;
    delete[] bt.de;
}

class NghiepVuThi : public NghiepVuChiXem {
private:
    DsLOP &dsLop;
    PTRSINHVIEN sv;
    MangMonHoc mang;
    treeMH Mon(int vt) const { return (vt >= 0 && vt < mang.soLuong) ? mang.ds[vt] : nullptr; }

public:
    NghiepVuThi(DsLOP &l, treeMH goc, PTRSINHVIEN s) : dsLop(l), sv(s) { DongBoMangMonHoc(mang, goc); }
    ~NghiepVuThi() { GiaiPhongMangMonHoc(mang); }

    int LayTongSo() override { return mang.soLuong; }
    void LayDuLieuBang(int vt, string cot[], TrangThaiDong &trangThai) override {
        trangThai = DONG_BINH_THUONG;
        treeMH mh = Mon(vt);
        if (mh == nullptr) return;
        cot[0] = SoSangChuoi(vt + 1);
        cot[1] = mh->data.MAMH;
        cot[2] = RutGon(mh->data.TENMH, 26);
        cot[3] = SoSangChuoi(DemNut(mh->data.dsCHT));
        if (TimDiemThi(sv, mh->data.MAMH) != nullptr) trangThai = DONG_KHOA;
    }

    void LayDuLieuForm(int vt, string giaTri[]) override {
        treeMH mh = Mon(vt);
        if (mh == nullptr) return;
        giaTri[0] = RutGon(mh->data.MAMH + " - " + mh->data.TENMH, 56);
        giaTri[1] = SoSangChuoi(DemNut(mh->data.dsCHT));
        giaTri[2] = "";
        giaTri[3] = "";
    }

    bool TruongBiKhoa(int vt, int idxTruong) override { (void)vt; return idxTruong < 2; }

    string KiemTraTruocSua(int vt) override {
        treeMH mh = Mon(vt);
        if (mh == nullptr) return "Khong tim thay mon hoc!";
        if (mh->data.dsCHT == nullptr) return "Mon hoc nay chua co cau hoi, khong the thi!";
        if (TimDiemThi(sv, mh->data.MAMH) != nullptr) return "Ban da thi mon nay, khong the thi lai!";
        return "";
    }

    string XuLySua(int vt, const string gt[]) override {
        string loi = KiemTraTruocSua(vt);
        if (!loi.empty()) return loi;
        treeMH mh = Mon(vt);
        int tong = DemNut(mh->data.dsCHT);

        const string &sc = gt[2], &sp = gt[3];
        if (sc.empty() || sp.empty()) return "Vui long nhap day du thong tin!";
        if (sc.length() > 4 || atoi(sc.c_str()) > tong)
            return "So cau thi toi da la " + SoSangChuoi(tong) + " (tong so cau cua mon)!";
        int soCau = atoi(sc.c_str());
        if (soCau <= 0) return "So cau thi phai lon hon 0!";
        if (sp.length() > 3 || atoi(sp.c_str()) > TOI_DA_PHUT_THI)
            return "So phut thi toi da la " + SoSangChuoi(TOI_DA_PHUT_THI) + "!";
        int soPhut = atoi(sp.c_str());
        if (soPhut <= 0) return "So phut thi phai lon hon 0!";

        LamBaiThi(dsLop, sv, mh, soCau, soPhut);
        return "";
    }
};

void ThiTracNghiem(treeMH &dsMonHoc, DsLOP &dsLop, PTRSINHVIEN svDangNhap) {
    if (svDangNhap == nullptr) return;

    DuLieuManHinhNhap data;
    data.tieuDe     = "Thiet lap Bai thi - Chon Mon thi";
    data.huongDanXem = "Enter: Thi";
    data.cheDoChiChon = true;
    data.dongMoiTrang = 20;

    data.coTimKiem = true;
    data.tenTimKiem = "ma mon hoc";
    KhoiTaoBang(data.bang, HANG_BANG, COT_BANG_TRAI, "mon hoc");
    ThemCot(data.bang, "STT", 5);
    ThemCot(data.bang, "MA MH", 14);
    ThemCot(data.bang, "TEN MON HOC", 37);
    ThemCot(data.bang, "SO CAU", 12);

    data.soTruongForm = 5;
    data.nhanForm[0] = "Mon thi (chon o bang ben trai):";   data.loaiTruong[0] = TRUONG_CHU;
    data.nhanForm[1] = "Tong so cau hoi hien co:";          data.loaiTruong[1] = TRUONG_CHU;
    data.nhanForm[2] = "So cau hoi thi:";                   data.loaiTruong[2] = TRUONG_SO;
    data.nhanForm[3] = "So phut thi (toi da 180):";         data.loaiTruong[3] = TRUONG_SO;
    data.nhanForm[4] = "  BAT DAU THI  ";                   data.loaiTruong[4] = TRUONG_NUT;
    data.maxLenTruong[3] = 3;
    KhoiTaoTrangThai(data.tt);

    NghiepVuThi nv(dsLop, dsMonHoc, svDangNhap);
    data.nghiepVu = &nv;

    ChayManHinhNhap(data);
}