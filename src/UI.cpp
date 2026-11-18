#include "mylib.h"
#include "KhaiBao.h"
#include "UI.h"

using namespace std;

int DONG_MOI_TRANG = 10;

static int PhienManHinh = 0;

static void DatMau(int nen, int chu) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)(((nen & 15) << 4) | (chu & 15)));
}

// in s voi mau, roi tra ve mau mac dinh
static void Xuat(const string &s, int nen, int chu) {
    DatMau(nen, chu);
    cout << s;
    cout.flush();
    DatMau(0, 15);
}

static const int CAO_O_FORM = 4;
static const int SO_LAN_DAT_KICH_THUOC = 40;

#ifndef ENABLE_WRAP_AT_EOL_OUTPUT
#define ENABLE_WRAP_AT_EOL_OUTPUT 0x0002
#endif

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

static DWORD cheDoXuatGoc = 0;
static bool  daLuuCheDoXuat = false;

static void KhoiPhucCheDoXuat() {
    if (daLuuCheDoXuat) SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), cheDoXuatGoc);
}

static void TatTuDongXuongDong() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (!GetConsoleMode(h, &mode)) return;
    if (!daLuuCheDoXuat) {
        cheDoXuatGoc = mode;
        daLuuCheDoXuat = true;
        atexit(KhoiPhucCheDoXuat);
    }
    SetConsoleMode(h, (mode & ~(DWORD)ENABLE_WRAP_AT_EOL_OUTPUT) | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

static HWND CuaSoWT() {
    HWND ch = GetConsoleWindow();
    if (ch == NULL) return NULL;
    HWND chu = GetAncestor(ch, GA_ROOTOWNER);
    char lop[64] = "";
    if (chu != NULL && GetClassNameA(chu, lop, sizeof(lop)) > 0 &&
        strcmp(lop, "CASCADIA_HOSTING_WINDOW_CLASS") == 0) return chu;
    return NULL;
}

static bool LaySoCotDong(int &cot, int &dong) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) return false;
    cot  = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    dong = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return true;
}

static bool KichThuocChinhXac() {
    int c, d;
    if (!LaySoCotDong(c, d)) return true;
    return c >= RONG_CONSOLE && c <= RONG_CONSOLE + 3 && d >= CAO_CONSOLE && d <= CAO_CONSOLE + 2;
}

static void BatDpiAware() {
    typedef BOOL (WINAPI *HamDpi)(void);
    HMODULE u = GetModuleHandleA("user32.dll");
    if (u == NULL) return;
    HamDpi f = (HamDpi)GetProcAddress(u, "SetProcessDPIAware");
    if (f != NULL) f();
}

static bool VungLamViec(HWND w, RECT &r) {
    HMONITOR m = MonitorFromWindow(w, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi; ZeroMemory(&mi, sizeof(mi)); mi.cbSize = sizeof(mi);
    if (m == NULL || !GetMonitorInfo(m, &mi)) return false;
    r = mi.rcWork;
    return true;
}

static bool DatKichThuocWT(HWND w, int maxLan) {
    if (w == NULL) return false;
    if (IsZoomed(w)) ShowWindow(w, SW_RESTORE);
    for (int lan = 0; lan < maxLan; lan++) {
        int c, d;
        if (!LaySoCotDong(c, d)) return false;

        RECT wr = {}, cr = {}, vl = {};
        if (!GetWindowRect(w, &wr) || !GetClientRect(w, &cr)) return false;
        bool coVung = VungLamViec(w, vl);
        int W = wr.right - wr.left, H = wr.bottom - wr.top;

        int newW = W, newH = H;
        if (!KichThuocChinhXac()) {
            double oC = (double)cr.right  / (c > 0 ? c : 1);
            double oD = (double)cr.bottom / (d > 0 ? d : 1);
            int dc = (RONG_CONSOLE + 1) - c, dd = (CAO_CONSOLE + 1) - d;
            int dx = (int)(dc * oC + (dc > 0 ? 0.5 : -0.5));
            int dy = (int)(dd * oD + (dd > 0 ? 0.5 : -0.5));
            if (dc != 0 && dx == 0) dx = dc > 0 ? 1 : -1;
            if (dd != 0 && dy == 0) dy = dd > 0 ? 1 : -1;
            newW = W + dx; newH = H + dy;
            if (coVung) {
                if (newW > vl.right - vl.left) newW = vl.right - vl.left;
                if (newH > vl.bottom - vl.top) newH = vl.bottom - vl.top;
            }
        }

        int x = wr.left, y = wr.top;
        if (coVung) {
            if (x + newW > vl.right)  x = vl.right - newW;
            if (y + newH > vl.bottom) y = vl.bottom - newH;
            if (x < vl.left) x = vl.left;
            if (y < vl.top)  y = vl.top;
        }

        bool doiSize = (newW != W || newH != H), doiViTri = (x != wr.left || y != wr.top);
        if (!doiSize && !doiViTri) return KichThuocChinhXac();
        SetWindowPos(w, NULL, x, y, newW, newH, SWP_NOZORDER | SWP_NOACTIVATE);
        Sleep(60);
        if (KichThuocChinhXac()) return true;
    }
    return KichThuocChinhXac();
}

static DWORD WINAPI LuongGiuKichThuoc(LPVOID tham) {
    HWND w = (HWND)tham;
    int thatBai = 0;
    while (true) {
        Sleep(500);
        if (KichThuocChinhXac() && !IsZoomed(w)) { thatBai = 0; continue; }
        if (thatBai >= 3) continue;
        if (DatKichThuocWT(w, 6)) thatBai = 0; else thatBai++;
    }
    return 0;
}

static void KhoiTaoWindowsTerminal(HWND wt) {
    BatDpiAware();
    bool dat = DatKichThuocWT(wt, SO_LAN_DAT_KICH_THUOC);
    bool tungLoi = !dat;

    while (!dat) {
        int c = 0, d = 0; LaySoCotDong(c, d);
        clrscr();
        gotoxy(2, 2);
        cout << "Khong dat duoc kich thuoc " << RONG_CONSOLE << "x" << CAO_CONSOLE
             << " trong Windows Terminal. Hien tai: " << c << "x" << d << " (cot x dong).";
        gotoxy(2, 3);
        cout << "Hay giam co chu (Ctrl + dau tru, hoac Settings > Defaults > Appearance > Font size), roi nhan phim bat ky.";
        gotoxy(2, 4);
        cout << "Nhan Esc de bo qua.";
        if (getch() == PHIM_ESC) break;
        dat = DatKichThuocWT(wt, SO_LAN_DAT_KICH_THUOC);
    }

    HANDLE t = CreateThread(NULL, 0, LuongGiuKichThuoc, (LPVOID)wt, 0, NULL);
    if (t != NULL) CloseHandle(t);
    if (tungLoi) clrscr();
}

void KhoiTaoConsole() {
    HWND wt = CuaSoWT();
    if (wt == NULL) return;
    TatTuDongXuongDong();
    KhoiTaoWindowsTerminal(wt);
}

int DocPhim() {
    int phim = getch();
    if (phim == 0 || phim == 224) {
       phim = -getch();
    }
    return phim;
}

bool CoKyTu(const string &s, char c) {
    for (int i = 0; i < (int)s.length(); i++)
        if (s[i] == c) return true;
    return false;
}

string KiemTraHopLeForm(const string gt[], int soTruong) {
    for (int i = 0; i < soTruong; i++)
        if (gt[i].empty()) return "Vui long nhap day du thong tin!";
    for (int i = 0; i < soTruong; i++)
        if (CoKyTu(gt[i], '|')) return "Khong duoc dung ky tu '|'!";
    return "";
}

string SoSangChuoi(int x) {
    if (x <= 0) return "0";
    string r = "";
    while (x > 0) {
        r = char('0' + x % 10) + r;
        x /= 10;
    }
    return r;
}

static string CatChuoi(const string &s, int n) {
    string r = "";
    for (int i = 0; i < (int)s.length() && i < n; i++) r += s[i];
    return r;
}

string RutGon(const string &s, int toiDa) {
    if ((int)s.length() <= toiDa) return s;
    if (toiDa <= 3) return s.substr(0, toiDa);
    return s.substr(0, toiDa - 3) + "...";
}

string CanGiua(const string &s, int rong) {
    int dai = (int)s.length();
    int trai = (dai >= rong) ? 0 : (rong - dai) / 2;
    string r = string(trai, ' ') + s;
    if ((int)r.length() < rong) r += string(rong - r.length(), ' ');
    return r;
}

static int GioiHanDoRongAnToan(int row, int col, int doRong) {
    if (row == CAO_CONSOLE - 1) {
        int conLai = (RONG_CONSOLE - 1) - col;
        if (conLai < 0) conLai = 0;
        if (doRong > conLai) doRong = conLai;
    }
    if (doRong < 0) doRong = 0;
    return doRong;
}

void VeDong(int row, int col, const string &noiDung, int doRong, int mauNen, int mauChu) {
    if (row < 0 || row >= CAO_CONSOLE || col < 0 || col >= RONG_CONSOLE) return;
    doRong = GioiHanDoRongAnToan(row, col, doRong);
    if (doRong <= 0) return;
    string dong = CatChuoi(noiDung, doRong);
    if ((int)dong.length() < doRong) dong += string(doRong - dong.length(), ' ');
    gotoxy(col, row);
    Xuat(dong, mauNen, mauChu);
}

void XoaVung(int startRow, int startCol, int width, int height, int mauNen, int mauChu) {
    if (startCol < 0) { width += startCol; startCol = 0; }
    if (startRow < 0) { height += startRow; startRow = 0; }
    if (startCol + width > RONG_CONSOLE) width = RONG_CONSOLE - startCol;
    if (startRow + height > CAO_CONSOLE) height = CAO_CONSOLE - startRow;
    if (width <= 0 || height <= 0) return;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD attr = (WORD)(((mauNen & 15) << 4) | (mauChu & 15));
    DWORD ghi;
    for (int i = 0; i < height; i++) {
        COORD toaDo = { (SHORT)startCol, (SHORT)(startRow + i) };
        FillConsoleOutputCharacterA(h, ' ', (DWORD)width, toaDo, &ghi);
        FillConsoleOutputAttribute(h, attr, (DWORD)width, toaDo, &ghi);
    }
}

void XoaThanManHinh() {
    PhienManHinh++;
    XoaVung(4, 0, RONG_CONSOLE, CAO_CONSOLE - 4);
}

void VeNut(int row, int col, const string &noiDung, bool highlight, int doRong, bool canGiua) {
    int nen = highlight ? 11 : 15;
    int chu = 0;
    string nd = canGiua ? CanGiua(TrimChuoi(noiDung), doRong) : "    " + noiDung;
    VeDong(row,     col, "", doRong, nen, chu);
    VeDong(row + 1, col, nd, doRong, nen, chu);
    VeDong(row + 2, col, "", doRong, nen, chu);
}

void VeTieuDe(const string &tieuDe, const string &phuDe) {
    VeDong(HANG_KE_TREN, 0, "", RONG_CONSOLE, 14, 6);
    VeDong(HANG_TIEU_DE, 0, CanGiua(tieuDe, RONG_CONSOLE), RONG_CONSOLE, 14, 6);
    VeDong(HANG_PHU_DE, 0, CanGiua(phuDe, RONG_CONSOLE), RONG_CONSOLE, 14, 4);
    VeDong(HANG_KE_DUOI, 0, "", RONG_CONSOLE, 14, 6);
}

static bool LaNut(const Truong &t) { return t.loai == TRUONG_NUT; }
static int SoDongHienThi(const Truong &t) { return t.soDong < 1 ? 1 : t.soDong; }
static int CaoOTruong(const Truong &t) {
    return LaNut(t) ? CHIEU_CAO_NUT : CAO_O_FORM + SoDongHienThi(t) - 1;
}

static Truong TaoTruong(LoaiTruong loai, const string &nhan, int row, int col, int rong) {
    Truong t; t.loai = loai; t.nhan = nhan; t.giaTri = ""; t.row = row; t.col = col; t.rong = rong; return t;
}

static bool LaChuCai(int c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
static bool LaChuSo(int c)  { return c >= '0' && c <= '9'; }

static string GiaTriMacDinh(const Truong &t) {
    if (t.loai == TRUONG_PHAI) return "Nam";
    if (t.loai == TRUONG_DAPAN) return "A";
    return "";
}

static bool ThemDauCach(string &s) {
    if (s.empty() || s[s.length() - 1] == ' ') return false;
    s += ' ';
    return true;
}

static bool LocPhimChuoi(LoaiTruong loai, string &s, int phim, int maxLen) {
    if (loai == TRUONG_NUT) return false;

    if (phim == PHIM_BACK) {
        if (loai == TRUONG_PHAI || loai == TRUONG_DAPAN || s.empty()) return false;
        s.erase(s.length() - 1);
        return true;
    }
    if (phim < 32 || phim > 126) return false;

    bool conCho = (int)s.length() < maxLen;
    bool dauTu  = s.empty() || s[s.length() - 1] == ' ';

    switch (loai) {
    case TRUONG_PHAI:
        if (phim != ' ') return false;
        s = (s == "Nam") ? "Nu" : "Nam";
        return true;

    case TRUONG_DAPAN: {
        if (phim != ' ') return false;
        char hienTai = s.empty() ? 'A' : (char)toupper((unsigned char)s[0]);
        s = string(1, (hienTai >= 'A' && hienTai < 'D') ? (char)(hienTai + 1) : 'A');
        return true;
    }
    case TRUONG_SO:
        if (!LaChuSo(phim) || !conCho) return false;
        s += (char)phim; return true;

    case TRUONG_MA:
        if (!(LaChuCai(phim) || LaChuSo(phim)) || !conCho) return false;
        s += (char)toupper(phim); return true;

    case TRUONG_TENLOP:
        if (!conCho) return false;
        if (phim == ' ') return ThemDauCach(s);
        if (!(LaChuCai(phim) || LaChuSo(phim))) return false;
        s += (char)(dauTu ? toupper(phim) : tolower(phim));
        return true;

    case TRUONG_HODEM:
        if (!conCho) return false;
        if (phim == ' ') return ThemDauCach(s);
        if (!LaChuCai(phim)) return false;
        s += (char)(dauTu ? toupper(phim) : tolower(phim));
        return true;

    case TRUONG_TEN:
        if (!LaChuCai(phim) || !conCho) return false;
        s += (char)(dauTu ? toupper(phim) : tolower(phim));
        return true;

    case TRUONG_DANGNHAP:
    case TRUONG_MATKHAU:
        if (phim == ' ' || !conCho) return false;
        s += (char)phim; return true;

    case TRUONG_NOIDUNG:
    case TRUONG_CHU:
    default:
        if (phim == '|' || !conCho) return false;
        s += (char)phim; return true;
    }
}

static string HienThiChuoi(LoaiTruong loai, const string &s) {
    return (loai == TRUONG_MATKHAU) ? string(s.length(), '*') : s;
}

static void InTaiCho(int x, int y, const string &s, int colorText, int colorBg) {
    if (y < 0 || y >= CAO_CONSOLE || x < 0 || x >= RONG_CONSOLE) return;
    if (y == CAO_CONSOLE - 1 && x >= RONG_CONSOLE - 1) return;
    gotoxy(x, y);
    Xuat(s, colorBg, colorText);
}

int ChiaDongVanBan(const string &s, int rongDong, string dong[], int toiDa) {
    if (rongDong < 1) rongDong = 1;
    if (toiDa < 1) return 0;
    int n = 0;
    auto them = [&](const string &t) { if (n < toiDa) dong[n++] = t; };

    size_t i = 0, tong = s.length();
    if (tong == 0) { them(""); return n; }
    while (i < tong && n < toiDa) {
        size_t conLai = tong - i;
        if (conLai <= (size_t)rongDong) { them(s.substr(i)); break; }

        size_t vtNgat = string::npos;
        for (size_t j = i + (size_t)rongDong; j > i; j--)
            if (s[j] == ' ') { vtNgat = j; break; }

        if (vtNgat != string::npos) {
            them(s.substr(i, vtNgat - i));
            i = vtNgat;
            while (i < tong && s[i] == ' ') i++;
            if (i >= tong) them("");
        } else {
            them(s.substr(i, rongDong));
            i += rongDong;
        }
    }
    return n;
}

// cac dong da ngat theo tu cua o nhap
static const int TOI_DA_DONG_CHU = 200;
struct DongChu {
    string d[TOI_DA_DONG_CHU];
    int n = 0;
};

static void ChiaDongTheoTu(const string &s, int rongDong, DongChu &dong) {
    dong.n = ChiaDongVanBan(s, rongDong, dong.d, TOI_DA_DONG_CHU);
}

static void TinhHienThiNhieuDong(const string &s, int rongDong, int soDong,
                                  DongChu &dong, int &hangCon, int &cotCon, int &dongDau) {
    ChiaDongTheoTu(s, rongDong, dong);
    hangCon = dong.n - 1;
    cotCon = (int)dong.d[hangCon].length();
    dongDau = hangCon - soDong + 1;
    if (dongDau < 0) dongDau = 0;
}

static bool TruongKhongGoChu(LoaiTruong loai) {
    return loai == TRUONG_PHAI || loai == TRUONG_DAPAN;
}

static void VeCuaSoNhieuDong(int x, int y, int rongDong, int soDong, const DongChu &dong,
                             int dongDau, LoaiTruong loai, int colorText, int colorBg) {
    for (int i = 0; i < soDong; i++) {
        int chiSo = dongDau + i;
        string noiDung = (chiSo >= 0 && chiSo < dong.n) ? dong.d[chiSo] : string("");
        string hien = HienThiChuoi(loai, noiDung);
        if ((int)hien.length() < rongDong) hien += string(rongDong - hien.length(), ' ');
        else if ((int)hien.length() > rongDong) hien = hien.substr(0, rongDong);
        InTaiCho(x, y + i, hien, colorText, colorBg);
    }
}

bool NhapTextNhieuDong(int x, int y, int rongDong, int soDong, int maxLen, string &outStr, LoaiTruong loai,
                       int colorText, int colorBg, int *phimThoat, std::function<void()> khiCoPhim,
                       std::function<void()> khiThayDoi) {
    if (rongDong < 1) rongDong = 1;
    if (soDong < 1) soDong = 1;
    if (maxLen < 0) maxLen = 0;
    if ((int)outStr.length() > maxLen) outStr.erase(maxLen);

    auto veLai = [&]() {
        AnHienConTro(false);
        DongChu dong; int hangCon, cotCon, dongDau;
        TinhHienThiNhieuDong(outStr, rongDong, soDong, dong, hangCon, cotCon, dongDau);
        VeCuaSoNhieuDong(x, y, rongDong, soDong, dong, dongDau, loai, colorText, colorBg);
    };
    auto datConTro = [&]() {
        if (TruongKhongGoChu(loai)) { AnHienConTro(false); return; }
        DongChu dong; int hangCon, cotCon, dongDau;
        TinhHienThiNhieuDong(outStr, rongDong, soDong, dong, hangCon, cotCon, dongDau);
        int hangHienThi = hangCon - dongDau;
        if (hangHienThi < 0) hangHienThi = 0;
        if (hangHienThi >= soDong) hangHienThi = soDong - 1;
        gotoxy(x + cotCon, y + hangHienThi);
        AnHienConTro(true);   // ve xong moi hien con tro
    };

    veLai();
    datConTro();

    bool ketQua = true;
    int phimKet;
    while (true) {
        int phim = DocPhim();
        if (khiCoPhim) { khiCoPhim(); khiCoPhim = nullptr; }

        if (phim == PHIM_ESC) { phimKet = phim; ketQua = false; break; }
        if (phim == PHIM_ENTER || LaPhimMuiTen(phim)) { phimKet = phim; break; }

        if (LocPhimChuoi(loai, outStr, phim, maxLen)) {
            veLai();
            if (khiThayDoi) khiThayDoi();
        }
        datConTro();
    }
    AnHienConTro(false);
    if (phimThoat) *phimThoat = phimKet;
    return ketQua;
}

static string CanLeHaiBen(const string &trai, const string &phai, int rong) {
    int rongTrai = rong - (int)phai.length();
    if (rongTrai < 0) rongTrai = 0;
    string s = RutGon(trai, rongTrai);
    if ((int)s.length() < rongTrai) s += string(rongTrai - s.length(), ' ');
    return s + phai;
}

static int RongNoiDungO(const Truong &t) {
    int w = t.rong - 4;
    return (w < 1) ? 1 : w;
}

static int MaxLenMacDinh(LoaiTruong loai) {
    switch (loai) {
    case TRUONG_MA: case TRUONG_TEN:           return 15;
    case TRUONG_DANGNHAP: case TRUONG_MATKHAU: return 20;
    case TRUONG_HODEM: case TRUONG_TENLOP:     return 50;
    case TRUONG_NOIDUNG:                       return 300;
    case TRUONG_SO:                            return 4;
    default:                                   return 0;
    }
}

static int GioiHanCung(const Truong &t) { return t.maxLen > 0 ? t.maxLen : MaxLenMacDinh(t.loai); }

static void VeNhanTruong(const Truong &t, bool chon) {
    if (t.khoa) chon = false;
    string nhan = (t.khoa && !t.anKhoaNhan) ? t.nhan + " (khoa)" : t.nhan;
    const int gh = GioiHanCung(t);
    string ghiChu = "";
    if (!t.khoa) {
        if (t.loai == TRUONG_PHAI) ghiChu = "Space: Nam/Nu";
        else if (t.loai == TRUONG_DAPAN) ghiChu = "Space: A/B/C/D";
        else if (gh > 0) ghiChu = SoSangChuoi((int)t.giaTri.length()) + "/" + SoSangChuoi(gh);
    }
    VeDong(t.row, t.col, ghiChu.empty() ? nhan : CanLeHaiBen(nhan, ghiChu, t.rong), t.rong, 0, chon ? 11 : 15);
}

static void VeOKhungNhieuDong(const Truong &t, bool chon) {
    const int soDong = SoDongHienThi(t);
    const int w = RongNoiDungO(t);
    const int mauVien = (chon && !t.khoa) ? 11 : 15;

    VeNhanTruong(t, chon);
    VeDong(t.row + 1, t.col, string(t.rong, '-'), t.rong, 0, mauVien);
    for (int i = 0; i < soDong; i++) {
        VeDong(t.row + 2 + i, t.col, "| ", 2, 0, mauVien);
        VeDong(t.row + 2 + i, t.col + t.rong - 2, " |", 2, 0, mauVien);
    }
    VeDong(t.row + 2 + soDong, t.col, string(t.rong, '-'), t.rong, 0, mauVien);

    DongChu dong; int hangCon, cotCon, dongDau;
    TinhHienThiNhieuDong(t.giaTri, w, soDong, dong, hangCon, cotCon, dongDau);
    VeCuaSoNhieuDong(t.col + 2, t.row + 2, w, soDong, dong, dongDau, t.loai, 15, 0);
}

static void VeTruong(const Truong &t, bool chon) {
    AnHienConTro(false);
    if (LaNut(t)) VeNut(t.row, t.col, t.nhan, chon, t.rong, t.canGiuaNut);
    else VeOKhungNhieuDong(t, chon);
}

static void VeBoDemKyTu(const Truong &t) {
    if (GioiHanCung(t) > 0) VeNhanTruong(t, true);
}

static bool NhapOTruong(Truong &t, int *phim, std::function<void()> khiCoPhim) {
    int w = RongNoiDungO(t), soDong = SoDongHienThi(t);
    int maxLen = GioiHanCung(t);
    if (maxLen <= 0) maxLen = w * soDong - 1;
    return NhapTextNhieuDong(t.col + 2, t.row + 2, w, soDong, maxLen, t.giaTri, t.loai,
                             15, 0, phim, khiCoPhim, [&t]() { VeBoDemKyTu(t); });
}

static void DatConTro(const Truong &t) {
    if (LaNut(t) || t.khoa || TruongKhongGoChu(t.loai)) { AnHienConTro(false); return; }    DongChu dong; int hangCon, cotCon, dongDau;
    TinhHienThiNhieuDong(t.giaTri, RongNoiDungO(t), SoDongHienThi(t), dong, hangCon, cotCon, dongDau);
    gotoxy(t.col + 2 + cotCon, t.row + 2 + hangCon - dongDau);
    AnHienConTro(true);
}

static void VeThongBao(int row, int col, const string &noiDung, bool isError = true, int rong = 0) {
    if (row < 0 || row >= CAO_CONSOLE || col < 0 || col >= RONG_CONSOLE) return;
    int toiDa = (rong > 0) ? rong : RONG_CONSOLE - col - 1;
    toiDa = GioiHanDoRongAnToan(row, col, toiDa);
    if (toiDa <= 0) return;
    VeDong(row, col, noiDung, toiDa, 0, noiDung.empty() ? 15 : (isError ? 12 : 10));
}

static int HangDuoiForm(const Truong ds[], int n) {
    int hang = 0;
    for (int i = 0; i < n; i++) {
        int duoi = ds[i].row + CaoOTruong(ds[i]);
        if (duoi > hang) hang = duoi;
    }
    return hang + 1;
}

// tim o ke tiep, bo qua o khoa
static int FocusKeTiep(const Truong ds[], int n, int focus, int buoc) {
    int f = focus;
    for (int i = 0; i < n; i++) {
        f = (f + buoc + n) % n;
        if (!ds[f].khoa) return f;
    }
    return focus;
}

int ChayForm(Truong ds[], int n, int &focus, const string &thongBao,
             int hangTBTuyChon, int cotTBTuyChon, int rongTB, bool nhanEsc,
             IKiemTraHetGio *kiemTraHetGio) {
    if (n <= 0) return -1;
    if (focus < 0 || focus >= n) focus = 0;
    int hangTB = HangDuoiForm(ds, n);
    int cotTB = ds[0].col;
    if (hangTBTuyChon >= 0) {
        hangTB = hangTBTuyChon;
        if (cotTBTuyChon >= 0) cotTB = cotTBTuyChon;
    }
    for (int i = 0; i < n; i++) VeTruong(ds[i], i == focus);
    bool coThongBao = !thongBao.empty();
    if (coThongBao) VeThongBao(hangTB, cotTB, thongBao, true, rongTB);
    DatConTro(ds[focus]);

    while (true) {
        int phim; int focusCu = focus;
        auto xoaThongBao = [&]() {
            if (coThongBao) { VeThongBao(hangTB, cotTB, "", true, rongTB); coThongBao = false; }
        };
        if (!LaNut(ds[focus]) && !ds[focus].khoa) {
            NhapOTruong(ds[focus], &phim, xoaThongBao);
        } else if (kiemTraHetGio) {
            while (!kbhit()) {
                if (kiemTraHetGio->HetGio()) { AnHienConTro(false); return -2; }
                Sleep(50);
            }
            phim = DocPhim();
        } else {
            phim = DocPhim();
        }
        xoaThongBao();
        if (phim == PHIM_ESC) {
            if (!nhanEsc) continue;   
            AnHienConTro(false); return -1;
        }
        else if (phim == PHIM_LEN || phim == PHIM_TRAI)   { focus = FocusKeTiep(ds, n, focus, -1); }
        else if (phim == PHIM_XUONG || phim == PHIM_PHAI) { focus = FocusKeTiep(ds, n, focus, 1); }
        else if (phim == PHIM_ENTER) {
            if (LaNut(ds[focus])) { AnHienConTro(false); return focus; }
            focus = FocusKeTiep(ds, n, focus, 1);
        }
        if (focus != focusCu) {
            VeTruong(ds[focusCu], false); VeTruong(ds[focus], true);
        }
        DatConTro(ds[focus]);
    }
}

int MenuTong(string options[], int soLuong, int startRow, int startCol, int focusBanDau) {
    if (soLuong > TOI_DA_TRUONG) soLuong = TOI_DA_TRUONG;
    int doRong = 0;
    for (int i = 0; i < soLuong; ++i)
        if ((int)options[i].length() > doRong) doRong = (int)options[i].length();
    doRong += 2 * 4;
    Truong ds[TOI_DA_TRUONG];
    for (int i = 0; i < soLuong; ++i)
    {
        ds[i] = TaoTruong(TRUONG_NUT, options[i], startRow + i * VE_NUT_MOI, startCol, doRong);
        ds[i].canGiuaNut = false;
    }
    if (soLuong <= 0) return -1;

    int focus = focusBanDau;
    AnHienConTro(false);
    for (int i = 0; i < soLuong; ++i) VeTruong(ds[i], i == focus);
    while (true) {
        int phim = DocPhim();
        int cu = focus;
        if (phim == PHIM_ENTER) return focus;
        else if (phim == PHIM_LEN || phim == PHIM_TRAI) focus = (focus - 1 + soLuong) % soLuong;
        else if (phim == PHIM_XUONG || phim == PHIM_PHAI) focus = (focus + 1) % soLuong;
        else continue;
        if (focus != cu) { VeTruong(ds[cu], false); VeTruong(ds[focus], true); }
    }
}

static const int HANG_HOP_THOAI = 10;
static const int CAO_HOP_THOAI  = 9;
static const int CAO_VUNG_TOI_DA = 14;

struct VungManHinh {
    SMALL_RECT rect;
    COORD kichThuoc;
    CHAR_INFO buf[RONG_CONSOLE * CAO_VUNG_TOI_DA];
    bool hopLe;
};

static void LuuVung(VungManHinh &v, int row, int col, int rong, int cao) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    v.kichThuoc.X = (SHORT)rong; v.kichThuoc.Y = (SHORT)cao;
    v.rect.Left = (SHORT)col;  v.rect.Top = (SHORT)row;
    v.rect.Right = (SHORT)(col + rong - 1); v.rect.Bottom = (SHORT)(row + cao - 1);
    COORD goc = {0, 0};
    SMALL_RECT r = v.rect;
    v.hopLe = (ReadConsoleOutputA(h, v.buf, v.kichThuoc, goc, &r) != 0);
}

static void KhoiPhucVung(const VungManHinh &v, int row, int col, int rong, int cao) {
    if (v.hopLe) {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD goc = {0, 0};
        SMALL_RECT r = v.rect;
        WriteConsoleOutputA(h, v.buf, v.kichThuoc, goc, &r);
    } else {
        XoaVung(row, col, rong, cao);
        PhienManHinh++;   // khong khoi phuc duoc: cac bang ve lai
    }
}

int HopThoai(const string &cauHoi, const string nut[], int soNut, int macDinh,
             IKiemTraHetGio *kiemTraHetGio) {
    if (soNut <= 0) return -1;
    if (soNut > TOI_DA_TRUONG) soNut = TOI_DA_TRUONG;

    const int doRongNut = 20, khoangCach = 4;
    int tongRongNut = soNut * doRongNut + (soNut - 1) * khoangCach;
    int rong = tongRongNut + 8;
    if ((int)cauHoi.length() + 8 > rong) rong = (int)cauHoi.length() + 8;
    if (rong < 44) rong = 44;
    if (rong > RONG_CONSOLE - 4) rong = RONG_CONSOLE - 4;
    int col = (RONG_CONSOLE - rong) / 2;
    int row = HANG_HOP_THOAI;

    static VungManHinh vung;
    LuuVung(vung, row, col, rong, CAO_HOP_THOAI);

    XoaVung(row, col, rong, CAO_HOP_THOAI, 14, 0);

    string q = RutGon(cauHoi, rong - 4);
    VeDong(row + 1, col + 1, CanGiua(q, rong - 2), rong - 2, 14, 6);

    Truong ds[TOI_DA_TRUONG];
    int batDauCol = col + (rong - tongRongNut) / 2;
    for (int i = 0; i < soNut; i++)
        ds[i] = TaoTruong(TRUONG_NUT, nut[i], row + 4, batDauCol + i * (doRongNut + khoangCach), doRongNut);

    int focus = macDinh;
    int kq = ChayForm(ds, soNut, focus, "", -1, -1, 0, false, kiemTraHetGio);
    KhoiPhucVung(vung, row, col, rong, CAO_HOP_THOAI);
    return kq;
}

bool XacNhan(const string &cauHoi) {
    string nut[2] = {"CO", "KHONG"};
    int chon = HopThoai(cauHoi, nut, 2, 0);
    return chon == 0;
}

KetQuaXacNhan XacNhanCoThoiGian(const string &cauHoi, IKiemTraHetGio *kiemTraHetGio) {
    string nut[2] = {"CO", "KHONG"};
    int chon = HopThoai(cauHoi, nut, 2, 0, kiemTraHetGio);
    if (chon == -2) return XN_HETGIO;
    return (chon == 0) ? XN_DONGY : XN_HUY;
}

void KhoiTaoBang(Bang &b, int row, int col, const string &donVi) {
    b.row = row; b.col = col; b.soCot = 0; b.soHuongDan = 0; b.donVi = donVi;
    for (int i = 0; i < TOI_DA_DONG_TRANG; i++) b.trangThai[i] = DONG_BINH_THUONG;
    b.phienVe = -1; b.khungDaVe = false;
    b.huongDanDaVe = ""; b.soTrangDaVe = ""; b.timKiemDaVe = "";
    for (int i = 0; i < TOI_DA_DONG_TRANG; i++) b.dongDaVe[i] = "";
}

// man hinh da bi xoa: bo bo nho dem bang
static void DongBoPhien(Bang &b) {
    if (b.phienVe == PhienManHinh) return;
    b.phienVe = PhienManHinh;
    b.khungDaVe = false;
    b.huongDanDaVe = ""; b.soTrangDaVe = ""; b.timKiemDaVe = "";
    for (int i = 0; i < TOI_DA_DONG_TRANG; i++) b.dongDaVe[i] = "";
}
void ThemCot(Bang &b, const string &tieuDe, int rong) {
    if (b.soCot >= TOI_DA_COT) return;
    b.cot[b.soCot].tieuDe = tieuDe; b.cot[b.soCot].rong = rong; b.soCot++;
}
static void ThemHuongDan(Bang &b, const string &dong) {
    if (b.soHuongDan >= TOI_DA_HUONGDAN) return;
    b.huongDan[b.soHuongDan] = dong; b.soHuongDan++;
}

// xep doan thanh cac dong <= rong (gom cum cach nhau 3 dau cach)
static void XepDoanHuongDan(Bang &b, const string &doan, int rong) {
    const string NGAN = "   ";
    string hienTai = "";
    size_t i = 0;
    while (true) {
        size_t j = doan.find(NGAN, i);
        string cum = TrimChuoi(doan.substr(i, (j == string::npos) ? string::npos : j - i));
        if (!cum.empty()) {
            if ((int)cum.length() > rong) {
                if (!hienTai.empty()) { ThemHuongDan(b, hienTai); hienTai = ""; }
                string phan[TOI_DA_HUONGDAN];
                int n = ChiaDongVanBan(cum, rong, phan, TOI_DA_HUONGDAN);
                for (int k = 0; k < n; k++) ThemHuongDan(b, phan[k]);
            }
            else if (hienTai.empty()) hienTai = cum;
            else if ((int)(hienTai.length() + NGAN.length() + cum.length()) <= rong) hienTai += NGAN + cum;
            else { ThemHuongDan(b, hienTai); hienTai = cum; }
        }
        if (j == string::npos) break;
        i = j + NGAN.length();
    }
    if (!hienTai.empty()) ThemHuongDan(b, hienTai);
}

static void ThemHuongDanXuongDong(Bang &b, const string &vanBan, int rong) {
    if (rong < 1) rong = 1;
    string sach = vanBan;
    for (size_t i = 0; i < sach.length(); i++)
        if ((unsigned char)sach[i] < 32 && sach[i] != '\n') sach[i] = ' ';
    size_t dau = 0;
    while (dau <= sach.length()) {
        size_t cuoi = sach.find('\n', dau);
        XepDoanHuongDan(b, sach.substr(dau, (cuoi == string::npos) ? string::npos : cuoi - dau), rong);
        if (cuoi == string::npos) break;
        dau = cuoi + 1;
    }
}

static bool HuongDanCoPhim(const Bang &b, const string &tok) {
    for (int i = 0; i < b.soHuongDan; i++) {
        const string &d = b.huongDan[i];
        for (size_t vt = d.find(tok); vt != string::npos; vt = d.find(tok, vt + 1))
            if (vt == 0 || d[vt - 1] == ' ' || d[vt - 1] == '/') return true;
    }
    return false;
}

void ChinhTrangThai(TrangThaiBang &t, int tong) {
    int soTrang = (tong + DONG_MOI_TRANG - 1) / DONG_MOI_TRANG;
    if (t.trang >= soTrang) t.trang = (soTrang > 0) ? soTrang - 1 : 0;
    if (t.trang < 0) t.trang = 0;
    int soDong = SoDongTrang(t, tong);
    if (t.chon >= soDong) t.chon = (soDong > 0) ? soDong - 1 : 0;
    if (t.chon < 0) t.chon = 0;
}
static int RongBang(const Bang &b) {
    int w = 2; for (int i = 0; i < b.soCot; i++) w += b.cot[i].rong; return w;
}
static bool CanGiuaCot(const string &tieuDe) {
    return tieuDe == "STT" || tieuDe == "ID" || tieuDe == "SO SV" ||
           tieuDe == "SO CAU HOI" || tieuDe == "SO CAU" || tieuDe == "DAP AN" || tieuDe == "DIEM";
}

static string Cell(const string &s, int rong, bool canGiua) {
    if (canGiua) return CanGiua(RutGon(s, rong), rong);
    string t = " " + RutGon(s, rong > 1 ? rong - 1 : 0);
    if ((int)t.length() < rong) t += string(rong - t.length(), ' ');
    return t;
}
static string NoiDungDong(const Bang &b, const string truong[], bool apDungCanGiua) {
    string r = "";
    for (int i = 0; i < b.soCot; i++)
        r += Cell(truong[i], b.cot[i].rong, apDungCanGiua && CanGiuaCot(b.cot[i].tieuDe));
    return r;
}
static const int HANG_LOI_BANG_TOI_DA      = CAO_CONSOLE - 2;
static const int HANG_LOI_FORM_TOI_DA      = CAO_CONSOLE - 2;
static const int HANG_XOA_HUONGDAN_TOI_DA  = CAO_CONSOLE - 3;

struct ViTriBang { int huongDan, thongBao; };
static ViTriBang TinhViTri(const Bang &b) {
    ViTriBang v; v.huongDan = b.row + 3 + DONG_MOI_TRANG + 2;
    v.thongBao = v.huongDan + TOI_DA_HUONGDAN + 1;   // co dinh (danh du 3 dong huong dan)
    if (v.thongBao > HANG_LOI_BANG_TOI_DA) v.thongBao = HANG_LOI_BANG_TOI_DA;
    return v;
}

static const char* CAC_TU_KHOA_PHIM[] = {
    "Len/Xuong/Trai/Phai:", "Len/Xuong/Enter:", "Len/Xuong:", "Trai/Phai:",
    "Delete:", "Enter:", "Space:", "F1:", "F2:", "F3:", "F4:", "Esc:", "1:", "A/B/C/D:"
};
static const int SO_TU_KHOA_PHIM = (int)(sizeof(CAC_TU_KHOA_PHIM) / sizeof(CAC_TU_KHOA_PHIM[0]));

void VeDongHuongDan(int row, int col, const string &s, int rong) {
    if (row < 0 || row >= CAO_CONSOLE || col < 0) return;
    int rongAnToan = GioiHanDoRongAnToan(row, col, rong);
    if (rongAnToan <= 0) return;
    string hien = CatChuoi(s, rongAnToan);
    for (size_t i = 0; i < hien.length(); i++)
        if ((unsigned char)hien[i] < 32) hien[i] = ' ';
    if ((int)hien.length() < rongAnToan) hien += string(rongAnToan - hien.length(), ' ');
    gotoxy(col, row);
    string thuong = "";
    size_t i = 0;
    while (i < hien.length()) {
        int khop = -1;
        if (i == 0 || hien[i - 1] == ' ') {
            for (int k = 0; k < SO_TU_KHOA_PHIM; k++) {
                size_t dai = strlen(CAC_TU_KHOA_PHIM[k]);
                if (hien.compare(i, dai, CAC_TU_KHOA_PHIM[k]) == 0) { khop = k; break; }
            }
        }
        if (khop >= 0) {
            if (!thuong.empty()) { Xuat(thuong, 0, 15); thuong = ""; }
            string tu = CAC_TU_KHOA_PHIM[khop];
            Xuat(tu, 0, 9);
            i += tu.length();
        } else {
            thuong += hien[i]; i++;
        }
    }
    if (!thuong.empty()) Xuat(thuong, 0, 15);
}

static void VeHuongDan(Bang &b, int rong) {
    DongBoPhien(b);
    ViTriBang v = TinhViTri(b);
    string khoa = SoSangChuoi(rong) + "," + SoSangChuoi(v.huongDan);
    for (int i = 0; i < b.soHuongDan; i++) khoa += "\n" + b.huongDan[i];
    if (b.huongDanDaVe == khoa) return;
    AnHienConTro(false);
    for (int i = 0; i < TOI_DA_HUONGDAN; i++) {
        int hang = v.huongDan + i;
        if (hang > HANG_XOA_HUONGDAN_TOI_DA) break;
        VeDongHuongDan(hang, b.col, i < b.soHuongDan ? b.huongDan[i] : string(""), rong);
    }
    b.huongDanDaVe = khoa;
}

static const int MAU_CHU_DANH_DAU = 10; //green 
static void VeDongBang(Bang &b, int i, bool coDuLieu, bool chon, bool khongInDoDongKhoa = false) {
    DongBoPhien(b);
    int rong = RongBang(b) - 2;
    string nd = coDuLieu ? NoiDungDong(b, b.o[i], true) : string(rong, ' ');
    TrangThaiDong st = coDuLieu ? b.trangThai[i] : DONG_BINH_THUONG;

    int mauNen = 0, mauChu = 15;
    bool gachNgang = false;
    if (chon) { mauNen = 8; mauChu = 15; }
    else if (st == DONG_CHUA_LUU) { mauNen = 10; mauChu = 0; }
    if (st == DONG_KHOA && !khongInDoDongKhoa) mauChu = 14;
    if (st == DONG_DANH_DAU) mauChu = MAU_CHU_DANH_DAU;
    if (st == DONG_DANH_DAU_XOA) { gachNgang = true; if (!chon) mauChu = 7; }

    string khoa = nd;
    khoa += '\x01'; khoa += (char)('A' + mauNen); khoa += (char)('A' + mauChu); khoa += gachNgang ? 'X' : '-';
    if (b.dongDaVe[i] == khoa) return;
    b.dongDaVe[i] = khoa;

    AnHienConTro(false);
    gotoxy(b.col, b.row + 3 + i);
    Xuat("|", 0, 15);
    DatMau(mauNen, mauChu);
    if (gachNgang) cout << "\x1b[9m";
    cout << nd;
    if (gachNgang) cout << "\x1b[29m";
    cout.flush();
    DatMau(0, 15);
    cout << "|";
    cout.flush();
}

static void VeKhungBang(Bang &b) {
    DongBoPhien(b);
    if (b.khungDaVe) return;   // khung khong doi: ve 1 lan
    int W = RongBang(b);
    string vien = "+" + string(W - 2, '-') + "+"; string tieuDe[TOI_DA_COT];
    AnHienConTro(false);
    for (int i = 0; i < b.soCot; i++) tieuDe[i] = b.cot[i].tieuDe;
    gotoxy(b.col, b.row);     Xuat(vien, 0, 15);
    gotoxy(b.col, b.row + 1); Xuat("|", 0, 15);
    Xuat(NoiDungDong(b, tieuDe, true), 0, 6);
    Xuat("|", 0, 15);
    gotoxy(b.col, b.row + 2); Xuat(vien, 0, 15);
    gotoxy(b.col, b.row + 3 + DONG_MOI_TRANG); Xuat(vien, 0, 15);
    b.khungDaVe = true;
}

static void VeSoTrang(Bang &b, int trang, int soTrang, int tong) {
    DongBoPhien(b);
    string tongTxt  = " Tong: " + SoSangChuoi(tong) + " " + b.donVi + " |";
    string trangTxt = " Trang: " + SoSangChuoi(tong == 0 ? 0 : trang + 1) + "/" + SoSangChuoi(soTrang) + " ";
    string khoa = tongTxt + trangTxt;
    if (b.soTrangDaVe == khoa) return;
    b.soTrangDaVe = khoa;

    int rongBang = RongBang(b);
    int hang = b.row + 3 + DONG_MOI_TRANG + 1;
    int colTong = b.col + rongBang - (int)(tongTxt.length() + trangTxt.length());
    if (colTong < b.col) colTong = b.col;
    // ghi 1 lan tu cot dau bang
    int rongTrai = (colTong - b.col) + (int)tongTxt.length();
    VeDong(hang, b.col, string(colTong - b.col, ' ') + tongTxt, rongTrai, 0, 15);
    VeDong(hang, b.col + rongTrai, trangTxt, (int)trangTxt.length(), 2, 15);
}
void VeNenManHinh(const string &phuDe) {
    XoaThanManHinh();
    VeTieuDe(TIEU_DE_CHUONG_TRINH, phuDe);
}

enum TrangThaiGiaoDien { MAN_HINH_XEM, MAN_HINH_CHO, FORM_THEM, FORM_SUA, FORM_SUA_CHO, TIM_KIEM, CHE_DO_XOA };

static bool DanhDauCo(const DsDanhDau &ds, int idx) {
    for (NutDanhDau *p = ds.dau; p != nullptr && p->idx >= idx; p = p->next)
        if (p->idx == idx) return true;
    return false;
}

static void DanhDauThem(DsDanhDau &ds, int idx) {
    NutDanhDau **p = &ds.dau;
    while (*p != nullptr && (*p)->idx > idx) p = &(*p)->next;
    NutDanhDau *nut = new NutDanhDau; nut->idx = idx; nut->next = *p;
    *p = nut; ds.n++;
}

static void DanhDauBo(DsDanhDau &ds, int idx) {
    NutDanhDau **p = &ds.dau;
    while (*p != nullptr && (*p)->idx > idx) p = &(*p)->next;
    if (*p == nullptr || (*p)->idx != idx) return;
    NutDanhDau *x = *p; *p = x->next; delete x; ds.n--;
}

static void DanhDauXoaHet(DsDanhDau &ds) { GiaiPhongDs(ds.dau); ds.n = 0; }

// ngu canh man hinh 1 bang + form
struct NguCanhNhap : BangLoc {
    TrangThaiGiaoDien trangThai;
    Truong form[TOI_DA_TRUONG];
    int focus, colForm;
    string thongBao; bool thongBaoLoi;
    bool lamSachMan;
    bool formDaVe;
    bool thoat;
    string tieuDeDangVe;
    TrangThaiGiaoDien trangThaiTruoc;
    int phien = -1;   // phien man hinh luc ve gan nhat
    string formKey;   // trang thai form luc ve gan nhat
};

static bool DangTrongForm(const NguCanhNhap &nc) {
    return nc.trangThai == FORM_THEM || nc.trangThai == FORM_SUA;
}

static bool FormKhoa(const NguCanhNhap &nc) {
    return nc.trangThai == FORM_SUA_CHO ||
           (nc.trangThai == TIM_KIEM && nc.trangThaiTruoc == FORM_SUA_CHO);
}

static const string NHAN_TIM_KIEM = "F4: Tim kiem ";

static int TongHienThi(const BangLoc &bl) {
    return bl.dangLoc ? bl.soLoc : bl.data->nghiepVu->LayTongSo();
}

static int ChiSoThat(const BangLoc &bl, int vt) {
    return bl.dangLoc ? bl.loc[vt] : vt;
}

struct KhoaLoc { const int *hang; const string *khoa; };

static bool NhoHon(const KhoaLoc &k, int a, int b) {
    if (k.hang[a] != k.hang[b]) return k.hang[a] < k.hang[b];
    return k.khoa[a] < k.khoa[b];
}

// merge sort tren mang chi so
static void SapXepLoc(int a[], int tam[], int l, int r, const KhoaLoc &k) {
    if (r - l < 2) return;
    int m = (l + r) / 2;
    SapXepLoc(a, tam, l, m, k);
    SapXepLoc(a, tam, m, r, k);
    int i = l, j = m, t = l;
    while (i < m && j < r) tam[t++] = NhoHon(k, a[j], a[i]) ? a[j++] : a[i++];
    while (i < m) tam[t++] = a[i++];
    while (j < r) tam[t++] = a[j++];
    for (t = l; t < r; t++) a[t] = tam[t];
}

static void TinhLoc(BangLoc &bl) {
    IManHinhNhap *nv = bl.data->nghiepVu;
    int tong = nv->LayTongSo();
    int cotTim = bl.data->cotTimKiem;
    if (cotTim < 0) cotTim = 0;
    if (cotTim > TOI_DA_COT - 1) cotTim = TOI_DA_COT - 1;
    delete[] bl.loc;
    bl.loc = new int[tong];
    bl.soLoc = 0;
    // bo khoang trang dau/cuoi, khong phan biet hoa/thuong
    string tuKhoaHoa = VietHoa(TrimChuoi(bl.tuKhoa));
    if (tuKhoaHoa.empty()) {
        for (int i = 0; i < tong; i++) bl.loc[bl.soLoc++] = i;
        return;
    }

    int *hang = new int[tong];
    string *khoa = new string[tong];
    string cot[TOI_DA_COT];
    for (int i = 0; i < tong; i++) {
        for (int c = 0; c < TOI_DA_COT; c++) cot[c] = "";
        nv->LayDuLieuTimKiem(i, cot);
        khoa[i] = VietHoa(cot[cotTim]);
        size_t vt = khoa[i].find(tuKhoaHoa);
        hang[i] = (vt == string::npos) ? -1 : (int)vt;
        if (hang[i] >= 0) bl.loc[bl.soLoc++] = i;
    }
    int *tam = new int[tong];
    KhoaLoc kl = { hang, khoa };
    SapXepLoc(bl.loc, tam, 0, bl.soLoc, kl);
    delete[] tam;
    delete[] khoa;
    delete[] hang;
}

static void VeOTimKiem(const BangLoc &bl, bool chon) {
    Bang &b = bl.data->bang;
    DongBoPhien(b);
    const string khoaVe = string("S") + (chon ? '1' : '0') + bl.tuKhoa;
    if (b.timKiemDaVe == khoaVe) return;
    b.timKiemDaVe = khoaVe;

    AnHienConTro(false);
    int row = b.row - 3, dai = (int)NHAN_TIM_KIEM.length();
    int col = b.col + dai, rong = RongBang(b) - dai;
    const int mauVien = chon ? 11 : 15;

    string dem = SoSangChuoi((int)bl.tuKhoa.length()) + "/" + SoSangChuoi(bl.toiDaLoc);
    VeDong(row - 1, col, CanLeHaiBen("", dem, rong), rong, 0, chon ? 11 : 15);
    VeDong(row + 1, b.col, "F4:", 3, 0, 9);
    VeDong(row + 1, b.col + 3, NHAN_TIM_KIEM.substr(3), dai - 3, 0, 15);
    VeDong(row, col, string(rong, '-'), rong, 0, mauVien);
    VeDong(row + 2, col, string(rong, '-'), rong, 0, mauVien);
    VeDong(row + 1, col, "|", 1, 0, mauVien);
    VeDong(row + 1, col + rong - 1, "|", 1, 0, mauVien);
    VeDong(row + 1, col + 1, "", 1, 0, 15);

    if (bl.tuKhoa.empty()) {
        string goiY = "(Tim kiem theo " + bl.data->tenTimKiem + ")";
        VeDong(row + 1, col + 2, goiY, rong - 3, 0, 8);
    } else {
        int len = (int)bl.tuKhoa.length(), hien = rong - 4;
        VeDong(row + 1, col + 2, len > hien ? bl.tuKhoa.substr(len - hien) : bl.tuKhoa, rong - 3, 0, 15);
    }
}

static void DatConTroTim(const BangLoc &bl) {
    const Bang &b = bl.data->bang;
    int hien = RongBang(b) - (int)NHAN_TIM_KIEM.length() - 4, dai = (int)bl.tuKhoa.length();
    gotoxy(b.col + (int)NHAN_TIM_KIEM.length() + 2 + (dai < hien ? dai : hien), b.row - 2);
    AnHienConTro(true);
}

static void NapDongBang(BangLoc &bl, int tong) {
    DuLieuManHinhNhap &d = *bl.data;
    for (int i = 0; i < DONG_MOI_TRANG; i++) {
        int vt = DauTrang(d.tt) + i;
        if (vt < tong) {
            int idx = ChiSoThat(bl, vt);
            d.nghiepVu->LayDuLieuBang(idx, d.bang.o[i], d.bang.trangThai[i]);
            if (DanhDauCo(bl.danhDau, idx))
                d.bang.trangThai[i] = bl.danhDauKieuXoa ? DONG_DANH_DAU_XOA : DONG_DANH_DAU;
        }
    }
}

static void VeCacDongBang(BangLoc &bl, int tong, bool hienChon) {
    DuLieuManHinhNhap &d = *bl.data;
    int soDong = SoDongTrang(d.tt, tong);
    AnHienConTro(false);
    for (int i = 0; i < DONG_MOI_TRANG; i++)
        VeDongBang(d.bang, i, i < soDong, hienChon && i == d.tt.chon && i < soDong, d.khongInDoDongKhoa);
    VeSoTrang(d.bang, d.tt.trang, (tong + DONG_MOI_TRANG - 1) / DONG_MOI_TRANG, tong);
}

static void XoaFormBenPhai(int colForm, int startRow, int soDong) {
    XoaVung(startRow, colForm, RONG_CONSOLE - colForm - 1, soDong);
}

static int HopThoaiXacNhanThoat() {
    string nut[3] = {"LUU VA THOAT", "THOAT", "HUY"};
    return HopThoai("Ban co muon luu thay doi truoc khi thoat?", nut, 3, 0);
}

static int HangThongBaoForm(const NguCanhNhap &nc) {
    int hang = HangDuoiForm(nc.form, nc.data->soTruongForm);
    return hang > HANG_LOI_FORM_TOI_DA ? HANG_LOI_FORM_TOI_DA : hang;
}

static void XoaThongBao(BangLoc &bl) {
    if (bl.hangTB >= 0) { VeThongBao(bl.hangTB, bl.cotTB, "", true, bl.rongTB); bl.hangTB = -1; }
    bl.noiDungTB = "";
}

static void HienThongBaoTai(BangLoc &bl, int hang, int cot, int rong, const string &msg, bool loi) {
    if (msg.empty()) { XoaThongBao(bl); return; }
    if (bl.hangTB == hang && bl.cotTB == cot && bl.rongTB == rong && bl.noiDungTB == msg && bl.loiTB == loi) return;
    if (bl.hangTB >= 0 && (bl.hangTB != hang || bl.cotTB != cot || bl.rongTB != rong)) XoaThongBao(bl);   // doi cho -> xoa cho cu
    bl.hangTB = hang; bl.cotTB = cot; bl.rongTB = rong; bl.noiDungTB = msg; bl.loiTB = loi;
    VeThongBao(hang, cot, msg, loi, rong);
}

static void HienThongBaoBang(BangLoc &bl, const string &msg, bool loi) {
    HienThongBaoTai(bl, TinhViTri(bl.data->bang).thongBao, bl.data->bang.col, bl.rongVung, msg, loi);
}

static void HienThongBao(NguCanhNhap &nc, const string &msg, bool loi) {
    if (!(loi && DangTrongForm(nc))) { HienThongBaoBang(nc, msg, loi); return; }
    // loi nhap form: hien duoi form
    HienThongBaoTai(nc, HangThongBaoForm(nc), nc.colForm, 0, msg, loi);
}

static void DatThongBao(NguCanhNhap &nc, const string &msg, bool loi) {
    nc.thongBao = msg; nc.thongBaoLoi = loi;
}

static void KhoiTaoNguCanh(NguCanhNhap &nc, DuLieuManHinhNhap &data) {
    if (data.soTruongForm > TOI_DA_TRUONG) data.soTruongForm = TOI_DA_TRUONG;
    nc.data = &data;
    nc.trangThai = data.batDauCheDoCho ? MAN_HINH_CHO : MAN_HINH_XEM;
    nc.focus = 0;
    nc.colForm = data.bang.col + RongBang(data.bang) + 5;
    nc.rongVung = nc.colForm - data.bang.col - 1;
    nc.thongBaoLoi = false;
    nc.lamSachMan = true; nc.formDaVe = false; nc.thoat = false;
    nc.trangThaiTruoc = nc.trangThai;
    nc.toiDaLoc = (data.loaiTimKiem == TRUONG_MA) ? 15 : 300;

    // tu tinh row tung Truong (nut cach 1 dong)
    int rowTuDong = HANG_TIMKIEM;
    for (int i = 0; i < data.soTruongForm; i++) {
        bool laNut = (data.loaiTruong[i] == TRUONG_NUT);
        int row = rowTuDong + (laNut ? 1 : 0);
        nc.form[i] = TaoTruong(data.loaiTruong[i], data.nhanForm[i],
                               row, nc.colForm, RONG_CONSOLE - nc.colForm - 5);
        nc.form[i].maxLen = data.maxLenTruong[i];
        nc.form[i].soDong = data.soDongTruong[i];
        nc.form[i].anKhoaNhan = data.anNhanKhoa[i];
        rowTuDong = row + CaoOTruong(nc.form[i]);
    }
}

static string HoaChuDau(const string &s) {
    if (s.empty()) return s;
    string r = s;
    r[0] = (char)toupper((unsigned char)r[0]);
    return r;
}

static string TieuDeHienTai(const NguCanhNhap &nc) {
    const DuLieuManHinhNhap &d = *nc.data;
    const string donVi = HoaChuDau(d.bang.donVi);

    switch (nc.trangThai) {
        case FORM_THEM:
            return d.tieuDe + ": Them " + donVi;
        case FORM_SUA:
        case FORM_SUA_CHO:
            return d.tieuDe + ": Sua " + donVi;
        case CHE_DO_XOA:
            return d.tieuDe + ": Xoa " + donVi;
        case TIM_KIEM:
        case MAN_HINH_XEM:
        case MAN_HINH_CHO:
        default:
            return d.tieuDe;
    }
}

static string KhoaVeForm(const NguCanhNhap &nc) {
    string k = SoSangChuoi(nc.focus) + (FormKhoa(nc) ? "K" : "M");
    for (int i = 0; i < nc.data->soTruongForm; i++) {
        k += '\x01'; k += nc.form[i].giaTri; k += nc.form[i].khoa ? '1' : '0';
    }
    return k;
}

static void VeManHinhNhap(NguCanhNhap &nc) {
    DuLieuManHinhNhap &d = *nc.data;
    AnHienConTro(false);
    int tong = TongHienThi(nc);
    ChinhTrangThai(d.tt, tong);

    nc.danhDauKieuXoa = (nc.trangThai == CHE_DO_XOA) ||
                        (nc.trangThai == TIM_KIEM && nc.trangThaiTruoc == CHE_DO_XOA);

    string tieuDe = TieuDeHienTai(nc);
    if (nc.lamSachMan) {
        nc.hangTB = -1; nc.noiDungTB = "";   
        VeNenManHinh(tieuDe);
        nc.formDaVe = false; nc.formKey = "";
        nc.tieuDeDangVe = tieuDe; nc.lamSachMan = false;
        nc.phien = PhienManHinh;
    } else {
        XoaThongBao(nc);
        if (nc.phien != PhienManHinh) {   
            nc.formDaVe = false; nc.formKey = ""; nc.tieuDeDangVe = ""; nc.phien = PhienManHinh;
        }
        if (tieuDe != nc.tieuDeDangVe) {
            VeTieuDe(TIEU_DE_CHUONG_TRINH, tieuDe);
            nc.tieuDeDangVe = tieuDe;
        }
    }
    if (d.coTimKiem) VeOTimKiem(nc, nc.trangThai == TIM_KIEM);

    d.bang.soHuongDan = 0;
    const int rongHD = nc.colForm - d.bang.col - 1;
    switch (nc.trangThai) {
        case MAN_HINH_XEM:
        {
            const string hdXem = d.huongDanXem.empty() ? string("1: Nhap lop   Enter: Nhap DSSV") : d.huongDanXem;
            ThemHuongDanXuongDong(d.bang, hdXem, rongHD);
            if (hdXem.find("Len/Xuong:") == string::npos)
                ThemHuongDanXuongDong(d.bang, "Len/Xuong: Chuyen dong   Trai/Phai: Chuyen trang   Esc: Thoat", rongHD);
        }
            break;
        case MAN_HINH_CHO:
            ThemHuongDanXuongDong(d.bang, "F1: Them   F2: Luu   F3: Sua   Delete: Xoa", rongHD);
            ThemHuongDanXuongDong(d.bang, "Len/Xuong: Chuyen dong   Trai/Phai: Chuyen trang   Esc: Thoat", rongHD);
            break;
        case FORM_THEM:
            ThemHuongDanXuongDong(d.bang, "Len/Xuong/Enter: Chuyen o   Trai/Phai: Chuyen trang   Esc: Thoat", rongHD);
            break;
        case FORM_SUA:
            ThemHuongDanXuongDong(d.bang, "Len/Xuong/Enter: Chuyen o   Esc: Thoat", rongHD);
            break;
        case FORM_SUA_CHO:
            ThemHuongDanXuongDong(d.bang, "Enter: Sua   Esc: Thoat", rongHD);
            ThemHuongDanXuongDong(d.bang, "Len/Xuong: Chuyen dong   Trai/Phai: Chuyen trang", rongHD);
            break;
        case TIM_KIEM:
            ThemHuongDanXuongDong(d.bang, "Len/Xuong: Chuyen dong   Trai/Phai: Chuyen trang   Enter: Chon   Esc: Thoat", rongHD);
            break;
        case CHE_DO_XOA:
            ThemHuongDanXuongDong(d.bang, "Enter: Danh dau/Bo danh dau   Delete: Xoa ("
                                          + SoSangChuoi(nc.danhDau.n) + ")   Esc: Huy", rongHD);
            ThemHuongDanXuongDong(d.bang, "Len/Xuong: Chuyen dong   Trai/Phai: Chuyen trang", rongHD);
            break;
    }

    NapDongBang(nc, tong);

    VeKhungBang(d.bang);
    VeHuongDan(d.bang, nc.colForm - d.bang.col);
    VeCacDongBang(nc, tong, nc.trangThai != FORM_THEM);

    if (DangTrongForm(nc)) {
        const string khoaForm = KhoaVeForm(nc);
        if (!nc.formDaVe || khoaForm != nc.formKey) {
            for (int i = 0; i < d.soTruongForm; i++) VeTruong(nc.form[i], i == nc.focus);
            nc.formKey = khoaForm;
            nc.formDaVe = true;
        }
    } else if (nc.formDaVe) {
        XoaFormBenPhai(nc.colForm, HANG_TIMKIEM, HangThongBaoForm(nc) - HANG_TIMKIEM + 1);
        nc.formDaVe = false; nc.formKey = "";
    }

    HienThongBao(nc, nc.thongBao, nc.thongBaoLoi);
    nc.thongBao = "";

    if (nc.trangThai == TIM_KIEM) DatConTroTim(nc);
    else if (DangTrongForm(nc) && !FormKhoa(nc)) DatConTro(nc.form[nc.focus]);
    else AnHienConTro(false);
}

static bool DiChuyenBang(BangLoc &bl, int phim, bool lenXuong) {
    DuLieuManHinhNhap &d = *bl.data;
    int tong = TongHienThi(bl);
    int soDong = SoDongTrang(d.tt, tong);
    int soTrang = (tong + DONG_MOI_TRANG - 1) / DONG_MOI_TRANG;

    if (lenXuong && (phim == PHIM_LEN || phim == PHIM_XUONG)) {
        if (soDong > 1) {
            int cu = d.tt.chon;
            d.tt.chon = (phim == PHIM_LEN) ? (cu - 1 + soDong) % soDong : (cu + 1) % soDong;
            VeDongBang(d.bang, cu, true, false, d.khongInDoDongKhoa);
            VeDongBang(d.bang, d.tt.chon, true, true, d.khongInDoDongKhoa);
        }
        return false;
    }
    if (phim == PHIM_TRAI && d.tt.trang > 0) { d.tt.trang--; return true; }
    if (phim == PHIM_PHAI && d.tt.trang < soTrang - 1) { d.tt.trang++; return true; }
    return false;
}

static void RoiCheDoCho(NguCanhNhap &nc) {
    if (nc.data->batDauCheDoCho) nc.thoat = true;
    else nc.trangThai = MAN_HINH_XEM;
}

static bool XuLyThoat(NguCanhNhap &nc) {
    IManHinhNhap *nv = nc.data->nghiepVu;
    if (!nv->CoThayDoi()) {
        RoiCheDoCho(nc); return true;
    }
    int chon = HopThoaiXacNhanThoat();
    if (chon == 0) {
        if (nv->XuLyLuu()) { nv->CapNhatSnapshot(); RoiCheDoCho(nc); }
        else DatThongBao(nc, "Loi ghi file! Du lieu chua duoc luu.", true);
    } else if (chon == 1) nv->KhoiPhucSnapshot(); RoiCheDoCho(nc);
    return true;
}

static void CapNhatBangLoc(BangLoc &bl) {
    DuLieuManHinhNhap &d = *bl.data;
    AnHienConTro(false);
    ChinhTrangThai(d.tt, bl.soLoc);
    NapDongBang(bl, bl.soLoc);
    VeCacDongBang(bl, bl.soLoc, true);
    VeOTimKiem(bl, true);
    if (bl.soLoc == 0) HienThongBaoBang(bl, "Khong co ket qua phu hop!", true);
}

static void BatDauLoc(BangLoc &bl) {
    DuLieuManHinhNhap &d = *bl.data;
    bl.ttTruocTim = d.tt;
    bl.tuKhoa = "";
    bl.dangLoc = true;
    TinhLoc(bl);
    d.tt.trang = 0; d.tt.chon = 0;
}

static void KetThucLoc(BangLoc &bl, bool chonDong) {
    DuLieuManHinhNhap &d = *bl.data;
    if (chonDong && bl.soLoc > 0) {
        int idx = bl.loc[ChiSoChon(d.tt)];
        d.tt.trang = idx / DONG_MOI_TRANG;
        d.tt.chon  = idx % DONG_MOI_TRANG;
    } else {
        d.tt = bl.ttTruocTim;
    }
    delete[] bl.loc;
    bl.loc = nullptr; bl.soLoc = 0;
    bl.dangLoc = false; bl.tuKhoa = "";
}

static void BatDauTim(NguCanhNhap &nc) {
    nc.trangThaiTruoc = nc.trangThai;
    BatDauLoc(nc);
    nc.trangThai = TIM_KIEM;
}

static void KetThucTim(NguCanhNhap &nc, bool chonDong) {
    KetThucLoc(nc, chonDong);
    nc.trangThai = nc.trangThaiTruoc;
}

static bool BamF4(NguCanhNhap &nc) {
    if (nc.data->nghiepVu->LayTongSo() <= 0) {
        DatThongBao(nc, "Danh sach trong!", true);
        return true;
    }
    BatDauTim(nc);
    return true;
}

static int XuLyPhimLoc(BangLoc &bl, int phim) {
    DuLieuManHinhNhap &d = *bl.data;
    if (phim == PHIM_ESC)   return 2;
    if (phim == PHIM_ENTER) return 3;
    if (LaPhimMuiTen(phim)) {
        if (DiChuyenBang(bl, phim, true)) return 1;
        DatConTroTim(bl);
        return 0;
    }
    string cu = bl.tuKhoa;
    if (LocPhimChuoi(d.loaiTimKiem, bl.tuKhoa, phim, bl.toiDaLoc) && bl.tuKhoa != cu) {
        AnHienConTro(false);
        if (TrimChuoi(bl.tuKhoa) == TrimChuoi(cu)) {
            // chi them/bo khoang trang: chi ve lai o tim kiem
            VeOTimKiem(bl, true);
        } else {
            TinhLoc(bl);
            d.tt.trang = 0; d.tt.chon = 0;
            CapNhatBangLoc(bl);
        }
    }
    DatConTroTim(bl);
    return 0;
}

static bool XuLyCheDoTim(NguCanhNhap &nc, int phim) {
    int kq = XuLyPhimLoc(nc, phim);
    if (kq == 2 || kq == 3) { KetThucTim(nc, kq == 3); return true; }
    return kq == 1;
}

// BANGLOC: BANG CO F4 + DANH DAU (NHIEU BANG)
static void XoaOTimKiem(Bang &b) {
    DongBoPhien(b);
    if (b.timKiemDaVe == "X") return;
    XoaVung(b.row - 4, b.col, RongBang(b), 4);
    b.timKiemDaVe = "X";
}

void GiaiPhongBangLoc(BangLoc &bl) {
    delete[] bl.loc;
    bl.loc = nullptr; bl.soLoc = 0;
    bl.dangLoc = false; bl.dangTim = false; bl.tuKhoa = "";
    DanhDauXoaHet(bl.danhDau);
}

void KhoiTaoBangLoc(BangLoc &bl, DuLieuManHinhNhap &data, int rongVung) {
    GiaiPhongBangLoc(bl);
    bl.data = &data;
    bl.rongVung = rongVung;
    bl.toiDaLoc = (data.loaiTimKiem == TRUONG_MA) ? 15 : 300;
    bl.ttTruocTim.trang = 0; bl.ttTruocTim.chon = 0;
    bl.hangTB = -1; bl.cotTB = 0; bl.rongTB = rongVung;
}

void VeBangLoc(BangLoc &bl, bool hoatDong, const string &huongDan) {
    DuLieuManHinhNhap &d = *bl.data;
    AnHienConTro(false);
    int tong = TongHienThi(bl);
    ChinhTrangThai(d.tt, tong);
    XoaThongBao(bl);

    if (d.coTimKiem) {
        if (hoatDong) VeOTimKiem(bl, bl.dangTim);
        else XoaOTimKiem(d.bang);
    }
    d.bang.soHuongDan = 0;
    ThemHuongDanXuongDong(d.bang, huongDan, bl.rongVung);
    NapDongBang(bl, tong);
    VeKhungBang(d.bang);
    VeHuongDan(d.bang, bl.rongVung + 1);
    VeCacDongBang(bl, tong, hoatDong);
    if (hoatDong && bl.dangTim) DatConTroTim(bl);
    else AnHienConTro(false);
}

// ve lai cac dong + so trang
void VeCacDongBangLoc(BangLoc &bl, bool hienChon) {
    int tong = TongHienThi(bl);
    ChinhTrangThai(bl.data->tt, tong);
    NapDongBang(bl, tong);
    VeCacDongBang(bl, tong, hienChon);
}

bool DiChuyenBangLoc(BangLoc &bl, int phim) { return DiChuyenBang(bl, phim, true); }

bool BatDauTimBangLoc(BangLoc &bl) {
    if (!bl.data->coTimKiem || bl.data->nghiepVu->LayTongSo() <= 0) return false;
    BatDauLoc(bl);
    bl.dangTim = true;
    return true;
}

bool XuLyPhimTimBangLoc(BangLoc &bl, int phim) {
    int kq = XuLyPhimLoc(bl, phim);
    if (kq == 2 || kq == 3) { KetThucLoc(bl, kq == 3); bl.dangTim = false; return true; }
    return kq == 1;
}

int ChiSoDongChon(const BangLoc &bl) {
    if (TongHienThi(bl) <= 0) return -1;
    return ChiSoThat(bl, ChiSoChon(bl.data->tt));
}

bool CoDanhDauBangLoc(const BangLoc &bl, int idx) { return DanhDauCo(bl.danhDau, idx); }

void DoiDanhDauBangLoc(BangLoc &bl, int idx) {
    if (DanhDauCo(bl.danhDau, idx)) DanhDauBo(bl.danhDau, idx);
    else DanhDauThem(bl.danhDau, idx);
}

// chep chi so da danh dau (tang dan), tra ve so luong
int LayDanhDauBangLoc(const BangLoc &bl, int ds[], int toiDa) {
    int vt = bl.danhDau.n - 1;   
    for (NutDanhDau *p = bl.danhDau.dau; p != nullptr; p = p->next, vt--)
        if (vt < toiDa) ds[vt] = p->idx;
    return bl.danhDau.n < toiDa ? bl.danhDau.n : toiDa;
}

void XoaDanhDauBangLoc(BangLoc &bl) { DanhDauXoaHet(bl.danhDau); }

static bool ThuMoFormSua(NguCanhNhap &nc);

// man hinh xem: chi xu ly phim co trong huong dan
static bool XuLyCheDoXem(NguCanhNhap &nc, int phim) {
    DuLieuManHinhNhap &d = *nc.data;
    IManHinhNhap *nv = d.nghiepVu;
    if (phim == PHIM_ESC) { nc.thoat = true; return false; }

    if (phim == PHIM_1) {   // phim 1 thay F1: vao man hinh cho
        if (d.cheDoChiChon || !HuongDanCoPhim(d.bang, "1:")) return false;
        nc.trangThai = MAN_HINH_CHO;
        return true;
    }
    if (phim == PHIM_F4 && d.coTimKiem) return BamF4(nc);
    if (phim == PHIM_ENTER) {
        if (!HuongDanCoPhim(d.bang, "Enter:")) return false;
        if (nv->LayTongSo() <= 0) return false;
        if (d.cheDoChiChon) return ThuMoFormSua(nc);
        nv->XuLyEnter(ChiSoChon(d.tt));
        nc.lamSachMan = true;
        return true;
    }
    return DiChuyenBang(nc, phim, true);
}

static void DatFormThem(NguCanhNhap &nc) {
    DuLieuManHinhNhap &d = *nc.data;
    string gt[TOI_DA_TRUONG]; int idx = 0;
    for (int i = 0; i < d.soTruongForm; i++)
        if (!LaNut(nc.form[i])) gt[idx++] = GiaTriMacDinh(nc.form[i]);
    d.nghiepVu->LayDuLieuFormThem(gt);
    idx = 0;
    for (int i = 0; i < d.soTruongForm; i++) {
        nc.form[i].khoa = false;
        if (LaNut(nc.form[i])) continue;
        nc.form[i].giaTri = gt[idx];
        nc.form[i].khoa = d.nghiepVu->TruongBiKhoaKhiThem(idx);
        idx++;
    }
    nc.formKey = "";   // ve de lai form: xoa chu cu, dem ky tu ve 0/Max
}

static int DauTienNhapDuoc(const NguCanhNhap &nc) {
    int f = 0;
    while (f < nc.data->soTruongForm - 1 && nc.form[f].khoa) f++;
    return f;
}

static void MoFormThem(NguCanhNhap &nc) {
    DatFormThem(nc);
    nc.focus = DauTienNhapDuoc(nc); nc.trangThai = FORM_THEM;
}

static void MoFormSua(NguCanhNhap &nc) {
    DuLieuManHinhNhap &d = *nc.data;
    int vt = ChiSoChon(d.tt);
    string gt[TOI_DA_TRUONG];
    d.nghiepVu->LayDuLieuForm(vt, gt);
    int idx = 0;
    for (int i = 0; i < d.soTruongForm; i++) {
        if (LaNut(nc.form[i])) { nc.form[i].khoa = false; continue; }
        nc.form[i].giaTri = gt[idx];
        nc.form[i].khoa = d.nghiepVu->TruongBiKhoa(vt, idx);
        idx++;
    }
    nc.focus = 0;
    while (nc.focus < d.soTruongForm - 1 && nc.form[nc.focus].khoa) nc.focus++;
    nc.formKey = "";   // ve de lai form
    nc.trangThai = FORM_SUA;
}

static bool ThuMoFormSua(NguCanhNhap &nc) {
    string loi = nc.data->nghiepVu->KiemTraTruocSua(ChiSoChon(nc.data->tt));
    if (!loi.empty()) DatThongBao(nc, loi, true);
    else MoFormSua(nc);
    return true;
}

static bool XuLyCheDoXoa(NguCanhNhap &nc, int phim) {
    DuLieuManHinhNhap &d = *nc.data;
    IManHinhNhap *nv = d.nghiepVu;
    int tong = nv->LayTongSo();

    if (phim == PHIM_ESC) { DanhDauXoaHet(nc.danhDau); nc.trangThai = MAN_HINH_CHO; return true; }

    if (phim == PHIM_DELETE) {
        if (nc.danhDau.n == 0) { DatThongBao(nc, "Chua danh dau " + d.bang.donVi + " nao!", true); return true; }

        int soSeXoa = nc.danhDau.n;
        if (!XacNhan("Ban co chac muon xoa " + SoSangChuoi(soSeXoa) + " " + d.bang.donVi + " khoi danh sach?"))
            return true; 

        string loiDau = "";
        for (NutDanhDau *it = nc.danhDau.dau; it != nullptr; it = it->next) {
            string loi = nv->XuLyXoa(it->idx);
            if (loiDau.empty()) loiDau = loi;
        }
        DanhDauXoaHet(nc.danhDau);
        ChinhTrangThai(d.tt, nv->LayTongSo());
        if (!loiDau.empty()) DatThongBao(nc, loiDau, true);
        return true;
    }
    if (phim == PHIM_F4 && d.coTimKiem) return BamF4(nc);

    if (phim == PHIM_ENTER) {
        if (tong <= 0) return false;
        int idx = ChiSoChon(d.tt);
        if (DanhDauCo(nc.danhDau, idx)) { DanhDauBo(nc.danhDau, idx); return true; }
        string loi = nv->KiemTraTruocXoa(idx);
        if (!loi.empty()) { DatThongBao(nc, loi, true); return true; }
        DanhDauThem(nc.danhDau, idx);
        return true;
    }

    return DiChuyenBang(nc, phim, true);
}

static bool XuLyCheDoCho(NguCanhNhap &nc, int phim) {
    DuLieuManHinhNhap &d = *nc.data;
    IManHinhNhap *nv = d.nghiepVu;
    int tong = nv->LayTongSo();

    if (phim == PHIM_ESC) return XuLyThoat(nc);
    if (phim == PHIM_F1)  { MoFormThem(nc); return true; }
    if (phim == PHIM_F2) {
        if (nv->XuLyLuu()) { nv->CapNhatSnapshot(); DatThongBao(nc, "Da luu du lie vao file!", false); }
        else DatThongBao(nc, "Loi ghi file! Du lieu chua duoc luu.", true);
        return true;
    }
    if (phim == PHIM_F3 && tong <= 0) { DatThongBao(nc, "Danh sach trong!", true); return true; }
    if (phim == PHIM_F3) {
        nc.trangThai = FORM_SUA_CHO;
        return true;
    }
    if (phim == PHIM_DELETE && tong <= 0) { DatThongBao(nc, "Danh sach trong!", true); return true; }
    if (phim == PHIM_DELETE) { DanhDauXoaHet(nc.danhDau); nc.trangThai = CHE_DO_XOA; return true; }
    if (phim == PHIM_F4 && d.coTimKiem) return BamF4(nc);
    return DiChuyenBang(nc, phim, true);
}

static bool XuLyFormSuaCho(NguCanhNhap &nc, int phim) {
    DuLieuManHinhNhap &d = *nc.data;
    if (phim == PHIM_ESC) { nc.trangThai = MAN_HINH_CHO; return true; }
    if (phim == PHIM_F4 && d.coTimKiem) return BamF4(nc);
    if (phim == PHIM_ENTER) {
        if (d.nghiepVu->LayTongSo() <= 0) return false;
        return ThuMoFormSua(nc);
    }
    return DiChuyenBang(nc, phim, true);
}

static void DoiFocus(NguCanhNhap &nc, int moi) {
    if (moi == nc.focus) return;
    VeTruong(nc.form[nc.focus], false);
    nc.focus = moi;
    VeTruong(nc.form[nc.focus], true);
    cout.flush();
    DatConTro(nc.form[nc.focus]);
}

static bool XacNhanForm(NguCanhNhap &nc) {
    DuLieuManHinhNhap &d = *nc.data;
    IManHinhNhap *nv = d.nghiepVu;
    bool them = (nc.trangThai == FORM_THEM);

    string gt[TOI_DA_TRUONG]; int dem = 0;
    for (int i = 0; i < d.soTruongForm; i++)
        if (!LaNut(nc.form[i])) gt[dem++] = TrimChuoi(nc.form[i].giaTri);

    string loi = them ? nv->XuLyThem(gt) : nv->XuLySua(ChiSoChon(d.tt), gt);
    if (!loi.empty()) { HienThongBao(nc, loi, true); return false; }

    if (them) {
        int tong = nv->LayTongSo();
        d.tt.trang = (tong > 0) ? (tong - 1) / DONG_MOI_TRANG : 0;
        d.tt.chon  = (tong > 0) ? (tong - 1) % DONG_MOI_TRANG : 0;
        DatFormThem(nc);
        nc.focus = DauTienNhapDuoc(nc);
    } else if (d.cheDoChiChon) {
        nc.thoat = true;
        return false;
    } else {
        nc.trangThai = FORM_SUA_CHO;
    }
    return true;
}

static bool XuLyFormNhap(NguCanhNhap &nc, int phim) {
    int n = nc.data->soTruongForm;
    if (phim == PHIM_ESC) {
        nc.trangThai = nc.data->cheDoChiChon ? MAN_HINH_XEM : MAN_HINH_CHO;
        return true;
    }
    if (n <= 0) return false;

    if (phim == PHIM_ENTER) {
        if (LaNut(nc.form[nc.focus])) return XacNhanForm(nc);
        DoiFocus(nc, FocusKeTiep(nc.form, n, nc.focus, 1)); return false;
    }
    if (phim == PHIM_LEN)   { DoiFocus(nc, FocusKeTiep(nc.form, n, nc.focus, -1)); return false; }
    if (phim == PHIM_XUONG) { DoiFocus(nc, FocusKeTiep(nc.form, n, nc.focus, 1)); return false; }
    if (phim == PHIM_TRAI || phim == PHIM_PHAI) {
        return (nc.trangThai == FORM_THEM) ? DiChuyenBang(nc, phim, false) : false;
    }
    return false;
}

void ChayManHinhNhap(DuLieuManHinhNhap &data) {
    const int dongMoiTrangCu = DONG_MOI_TRANG;
    DONG_MOI_TRANG = (data.dongMoiTrang < 1) ? 1 : (data.dongMoiTrang > TOI_DA_DONG_TRANG ? TOI_DA_DONG_TRANG : data.dongMoiTrang);
    data.nghiepVu->CapNhatSnapshot();
    NguCanhNhap nc;
    KhoiTaoNguCanh(nc, data);

    while (!nc.thoat) {
        VeManHinhNhap(nc);
        bool veLai = false;
        while (!veLai && !nc.thoat) {
            int phim;
            Truong &tf = nc.form[nc.focus];
            if (DangTrongForm(nc) && !LaNut(tf) && !tf.khoa) {
                NhapOTruong(tf, &phim, [&nc]() { XoaThongBao(nc); });
            } else {
                if (nc.trangThai != TIM_KIEM) AnHienConTro(false);
                phim = DocPhim();
                XoaThongBao(nc);
            }

            switch (nc.trangThai) {
                case MAN_HINH_XEM: veLai = XuLyCheDoXem(nc, phim); break;
                case MAN_HINH_CHO: veLai = XuLyCheDoCho(nc, phim); break;
                case FORM_THEM:
                case FORM_SUA:     veLai = XuLyFormNhap(nc, phim); break;
                case FORM_SUA_CHO: veLai = XuLyFormSuaCho(nc, phim); break;
                case TIM_KIEM:     veLai = XuLyCheDoTim(nc, phim); break;
                case CHE_DO_XOA:   veLai = XuLyCheDoXoa(nc, phim); break;
            }
        }
    }
    delete[] nc.loc;
    DanhDauXoaHet(nc.danhDau);
    AnHienConTro(false);
    DONG_MOI_TRANG = dongMoiTrangCu;
}