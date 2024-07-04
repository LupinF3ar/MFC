#pragma once

#include "afxwin.h"
#include "afxcmn.h"

class CTuan3Dlg : public CDialogEx
{
public:
    CTuan3Dlg(CWnd* pParent = nullptr); // Khai báo hàm khởi tạo

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TUAN3_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

protected:
    HICON m_hIcon;
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint(); // Khai báo phương thức OnPaint
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()

public:
    CEdit m_editFolderPath;
    CEdit m_editFileName;
    CButton m_btnSearch;
    CListCtrl m_listResult;

    afx_msg void OnBnClickedButtonSearch();
    void GetFileDetails(const CString& folderPath, const CString& fileName);
    CString GetFileOwner(const CString& filePath);
};
