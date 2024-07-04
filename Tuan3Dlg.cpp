#include "pch.h" // Ensure this is the first include
#include "framework.h"
#include "Tuan3.h"
#include "Tuan3Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <AclAPI.h>

CTuan3Dlg::CTuan3Dlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_TUAN3_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTuan3Dlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_FOLDER_PATH, m_editFolderPath);
    DDX_Control(pDX, IDC_EDIT_FILE_NAME, m_editFileName);
    DDX_Control(pDX, IDC_BUTTON_SEARCH, m_btnSearch);
    DDX_Control(pDX, IDC_LIST_RESULT, m_listResult);
}

BEGIN_MESSAGE_MAP(CTuan3Dlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CTuan3Dlg::OnBnClickedButtonSearch)
END_MESSAGE_MAP()

BOOL CTuan3Dlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetIcon(m_hIcon, TRUE);  // Set big icon
    SetIcon(m_hIcon, FALSE); // Set small icon

    // Initialize columns
    m_listResult.InsertColumn(0, _T("Tên file"), LVCFMT_LEFT, 150);
    m_listResult.InsertColumn(1, _T("Kích thước"), LVCFMT_LEFT, 100);
    m_listResult.InsertColumn(2, _T("Đường dẫn đầy đủ"), LVCFMT_LEFT, 250);
    m_listResult.InsertColumn(3, _T("Ngày sửa"), LVCFMT_LEFT, 120);
    m_listResult.InsertColumn(4, _T("Ngày tạo"), LVCFMT_LEFT, 120);
    m_listResult.InsertColumn(5, _T("Owner"), LVCFMT_LEFT, 100);

    return TRUE;
}

void CTuan3Dlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

HCURSOR CTuan3Dlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CTuan3Dlg::OnBnClickedButtonSearch()
{
    CString strFolderPath;
    m_editFolderPath.GetWindowText(strFolderPath);

    CString strFileName;
    m_editFileName.GetWindowText(strFileName);

    if (strFolderPath.IsEmpty() || strFileName.IsEmpty())
    {
        AfxMessageBox(_T("Đường dẫn thư mục và tên file không được để trống!"));
        return;
    }

    m_listResult.DeleteAllItems();  // Xóa kết quả cũ

    GetFileDetails(strFolderPath, strFileName);

    UpdateData(FALSE);
}

void CTuan3Dlg::GetFileDetails(const CString& folderPath, const CString& fileName)
{
    CString searchPath = folderPath + _T("\\") + fileName;
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(searchPath, &findData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        DWORD dwError = GetLastError();
        CString errorMsg;
        errorMsg.Format(_T("Không tìm thấy file! Mã lỗi: %lu"), dwError);
        AfxMessageBox(errorMsg);
        return;
    }

    do
    {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            continue;
        }

        CString filePath = folderPath + _T("\\") + findData.cFileName;
        CString fileSize;
        ULONGLONG sizeInBytes = findData.nFileSizeHigh;
        sizeInBytes <<= sizeof(findData.nFileSizeHigh) * 8;
        sizeInBytes |= findData.nFileSizeLow;
        fileSize.Format(_T("%llu"), sizeInBytes);

        FILETIME ftLastWriteTime = findData.ftLastWriteTime;
        FILETIME ftCreationTime = findData.ftCreationTime;
        SYSTEMTIME stLastWriteTime, stCreationTime;
        FileTimeToSystemTime(&ftLastWriteTime, &stLastWriteTime);
        FileTimeToSystemTime(&ftCreationTime, &stCreationTime);

        CString strLastWriteTime, strCreationTime;
        strLastWriteTime.Format(_T("%02d/%02d/%d %02d:%02d/%02d"),
            stLastWriteTime.wDay, stLastWriteTime.wMonth, stLastWriteTime.wYear,
            stLastWriteTime.wHour, stLastWriteTime.wMinute, stLastWriteTime.wSecond);
        strCreationTime.Format(_T("%02d/%02d/%d %02d:%02d/%02d"),
            stCreationTime.wDay, stCreationTime.wMonth, stCreationTime.wYear,
            stCreationTime.wHour, stCreationTime.wMinute, stCreationTime.wSecond);

        CString ownerName = GetFileOwner(filePath);

        int nIndex = m_listResult.InsertItem(0, findData.cFileName);
        m_listResult.SetItemText(nIndex, 1, fileSize);
        m_listResult.SetItemText(nIndex, 2, filePath);
        m_listResult.SetItemText(nIndex, 3, strLastWriteTime);
        m_listResult.SetItemText(nIndex, 4, strCreationTime);
        m_listResult.SetItemText(nIndex, 5, ownerName);

    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);
}

CString CTuan3Dlg::GetFileOwner(const CString& filePath)
{
    CString ownerName;
    PSECURITY_DESCRIPTOR pSD = NULL;

    if (GetNamedSecurityInfo(filePath, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, NULL, NULL, NULL, NULL, &pSD) == ERROR_SUCCESS)
    {
        PSID pOwner = NULL;
        BOOL bOwnerDefaulted = FALSE;

        if (GetSecurityDescriptorOwner(pSD, &pOwner, &bOwnerDefaulted) && pOwner != NULL)
        {
            TCHAR szOwner[MAX_PATH];
            DWORD dwOwnerSize = MAX_PATH;
            TCHAR szDomain[MAX_PATH];
            DWORD dwDomainSize = MAX_PATH;
            SID_NAME_USE sidUse;

            if (LookupAccountSid(NULL, pOwner, szOwner, &dwOwnerSize, szDomain, &dwDomainSize, &sidUse))
            {
                ownerName = szOwner;
            }
        }

        LocalFree(pSD);
    }

    return ownerName;
}
