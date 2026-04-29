
// TadpatraSplitter.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "TadpatraSplitter.h"
#include "TadpatraSplitterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTadpatraSplitterApp

BEGIN_MESSAGE_MAP(CTadpatraSplitterApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CTadpatraSplitterApp construction

CTadpatraSplitterApp::CTadpatraSplitterApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CTadpatraSplitterApp object

CTadpatraSplitterApp theApp;


// CTadpatraSplitterApp initialization

BOOL CTadpatraSplitterApp::InitInstance()
{
	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	//CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	CTadpatraSplitterDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	//if (pShellManager != nullptr)
	//{
	//	delete pShellManager;
	//}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

BOOL CTadpatraSplitterApp::ExitInstance()
{
	return TRUE;
}
