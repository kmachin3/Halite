﻿
//         Copyright Eóin O'Callaghan 2006 - 2008.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "stdAfx.hpp"

#include <boost/iterator/filter_iterator.hpp>
#include <winstl/controls/listview_sequence.hpp>

#include "Halite.hpp"

#include "HaliteListView.hpp"
#include "HaliteWindow.hpp"
#include "HaliteListViewDlg.hpp"

#include "halTorrent.hpp"

HaliteListViewCtrl::HaliteListViewCtrl(HaliteWindow& HalWindow) :
	halWindow_(HalWindow),
	iniClass("listviews/halite", "HaliteListView")
{		
	HalWindow.connectUiUpdate(bind(&HaliteListViewCtrl::uiUpdate, this, _1));
	
	std::vector<wstring> names;	
	wstring column_names = hal::app().res_wstr(LISTVIEW_ID_COLUMNNAMES);
	
	boost::split(names, column_names, boost::is_any_of(L";"));
	
	array<int, NumberOfColumns_s> widths = {100,110,60,60,60,42,45,61,45,45,45,45,45,45,45,45,45,45,45,45,45};
	array<int, NumberOfColumns_s> order = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	array<bool, NumberOfColumns_s> visible = {true,true,true,true,true,true,true,true,true,true,true,\
		true,true,true,true,true,true,true,true,true,true};
	
	SetDefaults(names, widths, order, visible);
	Load();
}

void HaliteListViewCtrl::OnShowWindow(UINT, INT)
{
	SetExtendedListViewStyle(WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_DOUBLEBUFFER);
	SetSortListViewExtendedStyle(SORTLV_USESHELLBITMAPS, SORTLV_USESHELLBITMAPS);
	
	ApplyDetails();
	
	SetColumnSortType(0, LVCOLSORT_CUSTOM, new ColumnAdapters::Name());
	SetColumnSortType(1, LVCOLSORT_CUSTOM, new ColumnAdapters::State());
	SetColumnSortType(2, LVCOLSORT_CUSTOM, new ColumnAdapters::Progress());
	SetColumnSortType(3, LVCOLSORT_CUSTOM, new ColumnAdapters::SpeedDown());
	SetColumnSortType(4, LVCOLSORT_CUSTOM, new ColumnAdapters::SpeedUp());
	SetColumnSortType(5, LVCOLSORT_CUSTOM, new ColumnAdapters::Peers());
	SetColumnSortType(6, LVCOLSORT_CUSTOM, new ColumnAdapters::Seeds());
	SetColumnSortType(7, LVCOLSORT_CUSTOM, new ColumnAdapters::ETA());
	SetColumnSortType(8, LVCOLSORT_CUSTOM, new ColumnAdapters::DistributedCopies());
	SetColumnSortType(9, LVCOLSORT_CUSTOM, new ColumnAdapters::Tracker());
	SetColumnSortType(10, LVCOLSORT_CUSTOM, new ColumnAdapters::UpdateTrackerIn());
	SetColumnSortType(11, LVCOLSORT_CUSTOM, new ColumnAdapters::Ratio());
	SetColumnSortType(12, LVCOLSORT_CUSTOM, new ColumnAdapters::TotalWanted());
	SetColumnSortType(13, LVCOLSORT_CUSTOM, new ColumnAdapters::Completed());
	SetColumnSortType(14, LVCOLSORT_CUSTOM, new ColumnAdapters::Remaining());
	SetColumnSortType(15, LVCOLSORT_CUSTOM, new ColumnAdapters::Downloaded());
	SetColumnSortType(16, LVCOLSORT_CUSTOM, new ColumnAdapters::Uploaded());
	SetColumnSortType(17, LVCOLSORT_CUSTOM, new ColumnAdapters::ActiveTime());
	SetColumnSortType(18, LVCOLSORT_CUSTOM, new ColumnAdapters::SeedingTime());
	SetColumnSortType(19, LVCOLSORT_CUSTOM, new ColumnAdapters::StartTime());
	SetColumnSortType(20, LVCOLSORT_CUSTOM, new ColumnAdapters::FinishTime());
}

void HaliteListViewCtrl::OnDestroy()
{
	saveSettings();
}

void HaliteListViewCtrl::saveSettings()
{
	GetListViewDetails();
	Save();
}

void HaliteListViewCtrl::uiUpdate(const hal::TorrentDetails& tD)
{
	hal::try_update_lock<listClass> lock(*this);
	if (lock) 
	{
	
	foreach (const hal::TorrentDetail_ptr td, tD.torrents()) 
	{
		LV_FINDINFO findInfo; 
		findInfo.flags = LVFI_STRING;
		findInfo.psz = const_cast<LPTSTR>(td->name().c_str());
		
		int itemPos = FindItem(&findInfo, -1);
		if (itemPos < 0)
			itemPos = AddItem(0, 0, td->name().c_str(), 0);

		for (size_t i=1; i<NumberOfColumns_s; ++i)
		{
			SetItemText(itemPos, i, getColumnAdapter(i)->print(td).c_str());
		}
	}
	
	int iCol = GetSortColumn();
	if (autoSort() && iCol >= 0 && iCol < m_arrColSortType.GetSize())
		DoSortItems(iCol, IsSortDescending());
	
	}
}

HaliteListViewCtrl::tD HaliteListViewCtrl::CustomItemConversion(LVCompareParam* param, int iSortCol)
{
	boost::array<wchar_t, MAX_PATH> buffer;
	GetItemText(param->iItem, 0, buffer.c_array(), buffer.size());		
	wstring torrent = buffer.data();
	
	return hal::bittorrent().torrentDetails().get(torrent);
}

LRESULT HaliteListViewCtrl::OnResume(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind((void (hal::bit::*)(const std::wstring&))&hal::bit::resumeTorrent, 
			&hal::bittorrent(), _1));
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind((void (hal::bit::*)(const std::wstring&))&hal::bit::pauseTorrent, 
			&hal::bittorrent(), _1));
	
	return 0;
}

LRESULT HaliteListViewCtrl::OnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind((void (hal::bit::*)(const std::wstring&))&hal::bit::stopTorrent, 
			&hal::bittorrent(), _1));

	return 0;
}

LRESULT HaliteListViewCtrl::OnRemoveFocused(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	hal::bittorrent().removeTorrent(hal::to_utf8(manager_.selected()));

	clearFocused();	
	return 0;
}

LRESULT HaliteListViewCtrl::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind((void (hal::bit::*)(const std::wstring&))&hal::bit::removeTorrent, 
			&hal::bittorrent(), _1));

	clearSelected();	
	return 0;
}

LRESULT HaliteListViewCtrl::OnRecheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
		bind((void (hal::bit::*)(const std::wstring&))&hal::bit::recheckTorrent, 
			&hal::bittorrent(), _1));	

	return 0;
}

LRESULT HaliteListViewCtrl::OnRemoveWipeFiles(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(MessageBox(hal::app().res_wstr(HAL_LISTVIEW_CONFIRMDELETE).c_str(), 
				hal::app().res_wstr(HAL_HALITE).c_str(), MB_YESNO) == IDYES)
	{
		std::for_each(manager().allSelected().begin(), manager().allSelected().end(),
			bind((void (hal::bit::*)(const std::wstring&))&hal::bit::removeTorrentWipeFiles, 
				&hal::bittorrent(), _1));
		
		clearSelected();
	}
	return 0;
}

LRESULT HaliteListViewCtrl::OnDownloadFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HAL_DEV_MSG(L"OnDownloadFolder");

	std::set<wpath> uniquePaths;

	for(std::set<wstring>::const_iterator i=manager().allSelected().begin(), e=manager().allSelected().end();
		i != e; ++i)
	{
		wpath saveDir = hal::bittorrent().get(*i).save_directory;		
		HAL_DEV_MSG(wformat(L"Name %1%, Save dir: %2%.") % *i % saveDir);

		uniquePaths.insert(saveDir);
	}

	SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };

	for(std::set<wpath>::const_iterator i=uniquePaths.begin(), e=uniquePaths.end();
		i != e; ++i)
	{	
		wstring p = (*i).file_string();

		HAL_DEV_MSG(wformat(L"Unique Save dir: %1%.") % p);

		sei.lpDirectory = p.c_str();
		sei.lpFile = p.c_str();
		sei.lpVerb = L"open";
		sei.nShow = true;

		if (!::ShellExecuteEx(&sei))
			HAL_DEV_MSG(L"Fail");
		else
			HAL_DEV_MSG(L"Success");
	}	

	return 0;
}

LRESULT HaliteListViewCtrl::OnEditFolders(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HAL_DEV_MSG(L"OnEditFolders");

	if (hal::bit::torrent t = hal::bittorrent().get(manager_.selected()))
	{
		wstring saveDirectory = static_cast<wpath>(t.save_directory).native_file_string();
		wstring moveToDirectory = static_cast<wpath>(t.move_to_directory).native_file_string();

		bool useMoveTo = (L"" != moveToDirectory);
		bool disableSaveDir = !t.in_session;

		HaliteListViewAdjustDlg addTorrent(hal::app().res_wstr(HAL_ADDT_TITLE), saveDirectory, moveToDirectory, 
			useMoveTo, disableSaveDir);	
		
		if (IDOK == addTorrent.DoModal())
		{
			if (!disableSaveDir) t.save_directory = saveDirectory;

			if (useMoveTo)
				t.move_to_directory = moveToDirectory;
			else
				t.move_to_directory = L"";
		}
	}

	return 0;
}

//LRESULT HaliteListViewCtrl::OnDeleteItem(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
//{
//	LPNMLISTVIEW pnmv=(LPNMLISTVIEW)pnmh;
//	T* pItem=(T*)GetItemData(pnmv->iItem);
//	ATLASSERT(pItem);
//	if (pItem)	// Delete attached structure
//		delete pItem;
//	return 0;
//}