#ifndef UI_H
#define UI_H

#include <string>
#include <functional>
using namespace std;

const int RONG_CONSOLE  = 145;
const int CAO_CONSOLE   = 44;
const int HANG_TIMKIEM = 8;
const int HANG_BANG     = HANG_TIMKIEM + 1;
const int COT_BANG_TRAI = 3;

const int PHIM_ENTER  = 13;
const int PHIM_ESC    = 27;
const int PHIM_BACK   = 8;
const int PHIM_LEN    = -72;
const int PHIM_XUONG  = -80;
const int PHIM_TRAI   = -75;
const int PHIM_PHAI   = -77;
const int PHIM_F1     = -59;
const int PHIM_F2     = -60;
const int PHIM_F3     = -61;
const int PHIM_F4     = -62;
const int PHIM_DELETE = -83;
const int PHIM_1      = '1';   

inline bool LaPhimMuiTen(int phim) {
    return phim == PHIM_LEN || phim == PHIM_XUONG || phim == PHIM_TRAI || phim == PHIM_PHAI;
}

const int CHIEU_CAO_NUT = 3;
const int VE_NUT_MOI = CHIEU_CAO_NUT + 2;

const int TOI_DA_TRUONG   = 12;
const int TOI_DA_COT      = 8;
const int TOI_DA_HUONGDAN = 3;
const int TOI_DA_DONG_TRANG = 30;
extern int DONG_MOI_TRANG;

const string TIEU_DE_CHUONG_TRINH = "HE THONG THI TRAC NGHIEM";

const int HANG_KE_TREN   = 0;
const int HANG_TIEU_DE   = 1;
const int HANG_PHU_DE    = 2;
const int HANG_KE_DUOI   = 3;

int DocPhim();
void KhoiTaoConsole();
bool CoKyTu(const string &s, char c);
string SoSangChuoi(int x);
string RutGon(const string &s, int toiDa);
string CanGiua(const string &s, int rong);

int ChiaDongVanBan(const string &s, int rongDong, string dong[], int toiDa);

string KiemTraHopLeForm(const string gt[], int soTruong);


inline string KiemTraTrungKhoa(const string gt[], int soTruong, bool daTonTai, const string &tenDoiTuong) {
    string loi = KiemTraHopLeForm(gt, soTruong);
    if (!loi.empty()) return loi;
    return daTonTai ? (tenDoiTuong + " da ton tai!") : "";
}

void VeDong(int row, int col, const string &noiDung, int doRong, int mauNen, int mauChu);
void VeDongHuongDan(int row, int col, const string &s, int rong);
void XoaVung(int startRow, int startCol, int width, int height, int mauNen = 0, int mauChu = 15);
void XoaThanManHinh();
void VeNut(int row, int col, const string &noiDung, bool highlight, int doRong, bool canGiua = true);
void VeTieuDe(const string &tieuDe, const string &phuDe);
void VeNenManHinh(const string &phuDe);

enum LoaiTruong { TRUONG_CHU, TRUONG_MATKHAU, TRUONG_NUT, TRUONG_PHAI,
                  TRUONG_MA, TRUONG_HODEM, TRUONG_TEN, TRUONG_DANGNHAP,
                  TRUONG_NOIDUNG, TRUONG_DAPAN, TRUONG_SO, TRUONG_TENLOP };

bool NhapTextNhieuDong(int x, int y, int rongDong, int soDong, int maxLen, string &outStr, LoaiTruong loai,
                       int colorText, int colorBg, int *phimThoat = nullptr,
                       std::function<void()> khiCoPhim = nullptr,
                       std::function<void()> khiThayDoi = nullptr);

struct Truong {
    LoaiTruong loai;
    string nhan;
    string giaTri;
    int row, col, rong;
    bool khoa;
    int soDong;
    int maxLen = 0;
    bool canGiuaNut = true;   
    bool anKhoaNhan = false;   

    Truong(LoaiTruong l = TRUONG_CHU, const string &n = "", const string &gt = "",
           int r = 0, int c = 0, int rg = 0, bool k = false)
        : loai(l), nhan(n), giaTri(gt), row(r), col(c), rong(rg), khoa(k), soDong(0) {}
};

struct IKiemTraHetGio {
    virtual ~IKiemTraHetGio() {}
    virtual bool HetGio() = 0;
};

int ChayForm(Truong ds[], int n, int &focus, const string &thongBao = "",
             int hangTB = -1, int cotTB = -1, int rongTB = 0, bool nhanEsc = true,
             IKiemTraHetGio *kiemTraHetGio = nullptr);
    
int MenuTong(string options[], int soLuong, int startRow, int startCol, int focusBanDau);
int HopThoai(const string &cauHoi, const string nut[], int soNut, int macDinh,
             IKiemTraHetGio *kiemTraHetGio = nullptr);
bool XacNhan(const string &cauHoi);
enum KetQuaXacNhan { XN_HUY, XN_DONGY, XN_HETGIO };

KetQuaXacNhan XacNhanCoThoiGian(const string &cauHoi, IKiemTraHetGio *kiemTraHetGio);

struct CotBang {
    string tieuDe;
    int rong;
};

enum TrangThaiDong { DONG_BINH_THUONG, DONG_CHUA_LUU, DONG_KHOA, DONG_DANH_DAU, DONG_DANH_DAU_XOA };

struct Bang {
    int row, col;
    CotBang cot[TOI_DA_COT];
    int soCot;
    string huongDan[TOI_DA_HUONGDAN];
    int soHuongDan;
    string donVi;
    string o[TOI_DA_DONG_TRANG][TOI_DA_COT];
    TrangThaiDong trangThai[TOI_DA_DONG_TRANG];

    int phienVe = -1;
    bool khungDaVe = false;
    string huongDanDaVe;
    string soTrangDaVe;
    string timKiemDaVe;   
    string dongDaVe[TOI_DA_DONG_TRANG];
};

void KhoiTaoBang(Bang &b, int row, int col, const string &donVi);
void ThemCot(Bang &b, const string &tieuDe, int rong);

struct TrangThaiBang {
    int trang;
    int chon;
};

inline void KhoiTaoTrangThai(TrangThaiBang &t) { t.trang = 0; t.chon = 0; }
inline int ChiSoChon(const TrangThaiBang &t) { return t.trang * DONG_MOI_TRANG + t.chon; }
inline int DauTrang(const TrangThaiBang &t) { return t.trang * DONG_MOI_TRANG; }
inline int SoDongTrang(const TrangThaiBang &t, int tong) {
    int n = tong - DauTrang(t);
    if (n > DONG_MOI_TRANG) n = DONG_MOI_TRANG;
    if (n < 0) n = 0;
    return n;
}
void ChinhTrangThai(TrangThaiBang &t, int tong);

struct IManHinhNhap {
    virtual ~IManHinhNhap() {}
    virtual int LayTongSo() = 0;
    virtual void LayDuLieuBang(int vt, string cot[], TrangThaiDong &trangThai) = 0;
    virtual void LayDuLieuForm(int vt, string giaTri[]) = 0;
    virtual void LayDuLieuTimKiem(int vt, string cot[]) { TrangThaiDong t; LayDuLieuBang(vt, cot, t); }

    virtual string XuLyThem(const string gt[]) = 0;
    virtual string XuLySua(int vt, const string gt[]) = 0;
    virtual string XuLyXoa(int vt) = 0;
    virtual string KiemTraTruocXoa(int vt) { return KiemTraTruocSua(vt); }

    virtual bool XuLyLuu() = 0;
    virtual bool CoThayDoi() = 0;
    virtual void KhoiPhucSnapshot() = 0;
    virtual void CapNhatSnapshot() = 0;

    virtual void XuLyEnter(int vt) { (void)vt; }
    virtual void LayDuLieuFormThem(string giaTri[]) { (void)giaTri; }
    virtual bool TruongBiKhoaKhiThem(int idxTruong) { (void)idxTruong; return false; }
    virtual string KiemTraTruocSua(int vt) { (void)vt; return ""; }
    virtual bool TruongBiKhoa(int vt, int idxTruong) { (void)vt; (void)idxTruong; return false; }
};

class NghiepVuChiXem : public IManHinhNhap {
public:
    void   LayDuLieuForm(int, string[]) override {}
    string XuLyThem(const string[]) override       { return "Man hinh nay chi de xem!"; }
    string XuLySua(int, const string[]) override   { return "Man hinh nay chi de xem!"; }
    string XuLyXoa(int) override                   { return "Man hinh nay chi de xem!"; }
    bool   XuLyLuu() override                      { return true; }
    bool   CoThayDoi() override                    { return false; }
    void   KhoiPhucSnapshot() override {}
    void   CapNhatSnapshot() override {}
};

typedef IManHinhNhap* PTRNGHIEPVU;

struct DuLieuManHinhNhap {
    string tieuDe;
    Bang bang;

    int soTruongForm;
    string nhanForm[TOI_DA_TRUONG];
    LoaiTruong loaiTruong[TOI_DA_TRUONG];
    bool anNhanKhoa[TOI_DA_TRUONG] = {};

    TrangThaiBang tt;
    PTRNGHIEPVU nghiepVu;

    string huongDanXem;
    bool batDauCheDoCho = false;
    bool cheDoChiChon = false;
    bool khongInDoDongKhoa = false;
    int dongMoiTrang = 20;
    int soDongTruong[TOI_DA_TRUONG] = {};
    int maxLenTruong[TOI_DA_TRUONG] = {};

    bool coTimKiem = false;
    int cotTimKiem = 1;
    string tenTimKiem = "ma";
    LoaiTruong loaiTimKiem = TRUONG_MA;
};

struct NutDanhDau { int idx; NutDanhDau *next; };
struct DsDanhDau { NutDanhDau *dau = nullptr; int n = 0; };

struct BangLoc {
    DuLieuManHinhNhap *data = nullptr;
    bool dangLoc = false;
    string tuKhoa;
    bool danhDauKieuXoa = false;   
    int *loc = nullptr;   
    int soLoc = 0;
    int toiDaLoc = 15;   
    TrangThaiBang ttTruocTim = {0, 0};
    DsDanhDau danhDau;
    int hangTB = -1, cotTB = 0, rongTB = 0;
    string noiDungTB;
    bool loiTB = true;
    int rongVung = 0;
    bool dangTim = false;  
};

// KhoiTaoBangLoc -> VeBangLoc -> xu ly phim -> GiaiPhong
void KhoiTaoBangLoc(BangLoc &bl, DuLieuManHinhNhap &data, int rongVung);
void GiaiPhongBangLoc(BangLoc &bl);
void VeBangLoc(BangLoc &bl, bool hoatDong, const string &huongDan);
void VeCacDongBangLoc(BangLoc &bl, bool hienChon);
bool DiChuyenBangLoc(BangLoc &bl, int phim);   // true: doi trang
bool BatDauTimBangLoc(BangLoc &bl);   // false: khong co o F4 / ds rong
bool XuLyPhimTimBangLoc(BangLoc &bl, int phim);   // true: can ve lai bang
int  ChiSoDongChon(const BangLoc &bl);   // chi so that cua dong chon, -1 neu rong
bool CoDanhDauBangLoc(const BangLoc &bl, int idx);
void DoiDanhDauBangLoc(BangLoc &bl, int idx);   // danh dau <-> bo danh dau
int  LayDanhDauBangLoc(const BangLoc &bl, int ds[], int toiDa);   // lay chi so da danh dau, tra ve so luong
void XoaDanhDauBangLoc(BangLoc &bl);

void ChayManHinhNhap(DuLieuManHinhNhap &data);

#endif