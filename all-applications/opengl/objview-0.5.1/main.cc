/**************************************************************************

    This is an example xclass OpenGL application.
    Copyright (C) 2000, 2001, Hector Peraza.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

**************************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>

#include <xclass/OXMsgBox.h>
#include <xclass/OXFileDialog.h>
#include <xclass/OXAboutDialog.h>
#include <xclass/version.h>

#include "main.h"
#include "view.h"

#include "tb-open.xpm"
#include "tb-save.xpm"
#include "tb-print.xpm"
#include "tb-leftside.xpm"
#include "tb-rightside.xpm"
#include "tb-topside.xpm"
#include "tb-bottomside.xpm"
#include "tb-frontside.xpm"
#include "tb-rearside.xpm"
#include "tb-perspective.xpm"
#include "tb-reset.xpm"
#include "tb-wireframe.xpm"

#include "menudef.h"
#include "toolbardef.h"

char *filetypes[] = { "Wavefront model files", "*.obj",
                      "All files",             "*",
                      NULL,                    NULL };

//----------------------------------------------------------------------

int main(int argc, char **argv) {
  OXClient *clientX = new OXClient(argc, argv);

  MainFrame *mainw = new MainFrame(clientX->GetRoot(), 10, 10);
  mainw->Resize(600, 400);

  if (argc > 1) mainw->ReadFile(argv[1]);

  mainw->MapWindow();

  clientX->Run();

  exit(0);
}

MainFrame::MainFrame(const OXWindow *p, int w, int h) :
  OXMainFrame(p, w, h) {

  _menuBarLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT | LHINTS_EXPAND_X, 
                                    0, 0, 1, 1);
  _menuBarItemLayout = new OLayoutHints(LHINTS_TOP | LHINTS_LEFT,
                                    0, 4, 0, 0);

  _menuFile = _MakePopup(&file_popup);
  _menuEdit = _MakePopup(&edit_popup);
  _menuView = _MakePopup(&view_popup);
  _menuHelp = _MakePopup(&help_popup);

  _menuFile->Associate(this);
  _menuEdit->Associate(this);
  _menuView->Associate(this);
  _menuHelp->Associate(this);

  //------ menu bar

  _menuBar = new OXMenuBar(this, 1, 1, HORIZONTAL_FRAME);
  _menuBar->AddPopup(new OHotString("&File"), _menuFile, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Edit"), _menuEdit, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&View"), _menuView, _menuBarItemLayout);
  _menuBar->AddPopup(new OHotString("&Help"), _menuHelp, _menuBarItemLayout);

  AddFrame(_menuBar, _menuBarLayout);

  //---- toolbar

  _toolBarSep = new OXHorizontal3dLine(this);

  _toolBar = new OXToolBar(this);
  _toolBar->AddButtons(tb_data);

  AddFrame(_toolBarSep, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X));
#ifdef _USE_FLAT_BUTTONS_
  AddFrame(_toolBar, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 
                                      0, 0, 0, 0));
#else
  AddFrame(_toolBar, new OLayoutHints(LHINTS_TOP | LHINTS_EXPAND_X, 
                                      0, 0, 3, 3));
#endif

  //------ GL frame

  OXCompositeFrame *glf = new OXCompositeFrame(this, 10, 10,
                                               SUNKEN_FRAME | DOUBLE_BORDER);
  AddFrame(glf, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  _glView = new GLView(glf);
  glf->AddFrame(_glView, new OLayoutHints(LHINTS_EXPAND_X | LHINTS_EXPAND_Y));

  //------ status bar

  _statusBar = new OXStatusBar(this);
  AddFrame(_statusBar, new OLayoutHints(LHINTS_BOTTOM | LHINTS_EXPAND_X,
                                     0, 0, 3, 0));

  _statusBar->SetText(0, new OString("Ready"));

  SetWindowTitle(NULL);
  SetClassHints("XCLASS", "XCLASS");

  MapSubwindows();
}

MainFrame::~MainFrame() {
  delete _menuBarLayout;
  delete _menuBarItemLayout;

  delete _menuFile;
  delete _menuEdit;
  delete _menuView;
  delete _menuHelp;
}

int MainFrame::CloseWindow() {
  return OXMainFrame::CloseWindow();
}

OXPopupMenu *MainFrame::_MakePopup(struct _popup *p) {

  OXPopupMenu *popup = new OXPopupMenu(_client->GetRoot());

  for (int i=0; p->popup[i].name != NULL; ++i) {
    if (strlen(p->popup[i].name) == 0) {
      popup->AddSeparator();
    } else {
      if (p->popup[i].popup_ref == NULL) {
        popup->AddEntry(new OHotString(p->popup[i].name), p->popup[i].id);
      } else {
        struct _popup *p1 = p->popup[i].popup_ref;
        popup->AddPopup(new OHotString(p->popup[i].name), p1->ptr);
      }
      if (p->popup[i].state & MENU_DISABLED) popup->DisableEntry(p->popup[i].id);
      if (p->popup[i].state & MENU_CHECKED) popup->CheckEntry(p->popup[i].id);
      if (p->popup[i].state & MENU_RCHECKED) popup->RCheckEntry(p->popup[i].id,
                                                                p->popup[i].id,
                                                                p->popup[i].id);
    }
  }
  p->ptr = popup;

  return popup;
}

int MainFrame::ProcessMessage(OMessage *msg) {
  OWidgetMessage *wmsg = (OWidgetMessage *) msg;

  switch (msg->type) {

    case MSG_MENU:
    case MSG_BUTTON:
      switch (msg->action) {

        case MSG_CLICK:
          switch (wmsg->id) {

            //--------------------------------------- File

            case M_FILE_OPEN:
              DoOpen();
              break;

            case M_FILE_SAVE:
              DoSave(NULL);
              break;

            case M_FILE_SAVEAS:
              DoSave(NULL);
              break;

            case M_FILE_PRINT:
              DoPrint();
              break;

            case M_FILE_EXIT:
              CloseWindow();
              break;

            //--------------------------------------- Edit

            case M_EDIT_CUT:
              break;

            case M_EDIT_COPY:
              break;

            case M_EDIT_PASTE:
              break;

            //--------------------------------------- View

            case M_VIEW_LEFTSIDE:
              _glView->LeftSideView();
              break;

            case M_VIEW_RIGHTSIDE:
              _glView->RightSideView();
              break;

            case M_VIEW_TOPSIDE:
              _glView->TopView();
              break;

            case M_VIEW_BOTTOMSIDE:
              _glView->BottomView();
              break;

            case M_VIEW_FRONTSIDE:
              _glView->FrontView();
              break;

            case M_VIEW_REARSIDE:
              _glView->RearView();
              break;

            case M_VIEW_PERSPECTIVE:
              _glView->PerspectiveView();
              break;

            case M_VIEW_RESET:
              _glView->ResetView();
              break;

            case M_VIEW_TOOLBAR:
              DoToggleToolBar();
              break;

            case M_VIEW_STATUSBAR:
              DoToggleStatusBar();
              break;

            case M_VIEW_SCALE:
              break;

            case M_VIEW_SMTANGLE:
              break;

            case M_VIEW_WELDDIST:
              break;

            case M_VIEW_BBOX:
              _glView->BoundingBox = !_glView->BoundingBox;
              _glView->NeedRedraw();
              _menuView->CheckEntry(M_VIEW_BBOX, _glView->BoundingBox);
              break;

            case M_VIEW_MATNONE:
            case M_VIEW_MATCOLOR:
            case M_VIEW_MATMAT:
              break;

            case M_VIEW_TWOSIDELGT:
              _glView->TwoSidedLighting = !_glView->TwoSidedLighting;
              _glView->NeedRedraw();
              _menuView->CheckEntry(M_VIEW_TWOSIDELGT, _glView->TwoSidedLighting);
              break;

            case M_VIEW_STATS:
              break;

            case M_VIEW_CULLFACE:
              _glView->CullFaces = !_glView->CullFaces;
              _glView->NeedRedraw();
              _menuView->CheckEntry(M_VIEW_CULLFACE, _glView->CullFaces);
              break;

            case M_VIEW_WIREFRAME:
              _glView->Wireframe = !_glView->Wireframe;
              _glView->NeedRedraw();
              _menuView->CheckEntry(M_VIEW_WIREFRAME, _glView->Wireframe);
              break;

            case M_VIEW_REVWINDING:
              _glView->ReverseWinding();
              break;

            //--------------------------------------- Help

            case M_HELP_CONTENTS:
              break;

            case M_HELP_SEARCH:
              break;

            case M_HELP_ABOUT:
              DoAbout();
              break;

            default:
              break;

          } // switch (wmsg->id)
          break;

        default:
          break;

      } // switch (msg->action)
      break;

    default:
      break;

  } // switch (msg->type)

  return True;
}

//----------------------------------------------------------------------

void MainFrame::SetWindowTitle(char *title) {
  static char *pname = "Wavefront OBJ model viewer";

  if (title) {
    char *wname = new char[strlen(title) + strlen(pname) + 4];
    sprintf(wname, "%s - %s", pname, title);
    SetWindowName(wname);
    delete wname;
  } else {
    SetWindowName(pname);
  }
}

void MainFrame::UpdateStatus() {
  char tmp[1024];

  strcpy(tmp, "Ready");
  _statusBar->SetText(0, new OString(tmp));
}

//----------------------------------------------------------------------

void MainFrame::ReadFile(char *fname) {
  if (!fname) return;

  if (!_glView->LoadModel(fname)) {

    char tmp[PATH_MAX];
    sprintf(tmp, "Failed to load model from file \"%s\"", fname);
    new OXMsgBox(_client->GetRoot(), this, new OString("Load Model"),
                 new OString(tmp), MB_ICONSTOP, ID_OK);
    return;
  }

  _glView->ResetView();
}

void MainFrame::WriteFile(char *fname) {
  FILE *fp;

  if (!fname) return;

  if ((fp = fopen(fname, "w")) == NULL) {

    char tmp[PATH_MAX];
    sprintf(tmp, "Could not create output file \"%s\": %s.",
                 fname, strerror(errno));
    new OXMsgBox(_client->GetRoot(), this, new OString("File Open"),
                 new OString(tmp), MB_ICONSTOP, ID_OK);
    return;

  } else {

    // Add your code to save the file here

    fclose(fp);
  }
}

void MainFrame::DoOpen() {
  OFileInfo fi;
  FILE *fp;

  fi.MimeTypesList = NULL;
  fi.file_types = filetypes;

  new OXFileDialog(_client->GetRoot(), this, FDLG_OPEN, &fi);
  if (fi.filename) {
    ReadFile(fi.filename);
  }
}

void MainFrame::DoSave(char *fname) {
  int retc;
  OFileInfo fi;
  FILE *fp;

  fi.MimeTypesList = NULL;
  fi.file_types = filetypes;

  new OXFileDialog(_client->GetRoot(), this, FDLG_SAVE, &fi);
  if (fi.filename) {

    // check whether the file already exists, and ask
    // permission to overwrite if so.

    if (access(fi.filename, F_OK) == 0) {
      new OXMsgBox(_client->GetRoot(), this,
            new OString("Save"),
            new OString("A file with the same name already exists. Overwrite?"),
            MB_ICONQUESTION, ID_YES | ID_NO, &retc);
      if (retc == ID_NO) return;
    }

    WriteFile(fi.filename);
  }
}

void MainFrame::DoPrint() {

  // Put your printing code here...

}

void MainFrame::DoToggleToolBar() {
  if (_toolBar->IsVisible()) {
    HideFrame(_toolBar);
    HideFrame(_toolBarSep);
    _menuView->UnCheckEntry(M_VIEW_TOOLBAR);
  } else {
    ShowFrame(_toolBar);
    ShowFrame(_toolBarSep);
    _menuView->CheckEntry(M_VIEW_TOOLBAR);
  }
}

void MainFrame::DoToggleStatusBar() {
  if (_statusBar->IsVisible()) {
    HideFrame(_statusBar);
    _menuView->UnCheckEntry(M_VIEW_STATUSBAR);
  } else {
    ShowFrame(_statusBar);
    _menuView->CheckEntry(M_VIEW_STATUSBAR);
  }
}

void MainFrame::DoAbout() {
  OAboutInfo info;
  
  info.wname = "About Wavefront OBJ Model Viewer";
  info.title = "Wavefront OBJ Model Viewer\n"
               "Compiled with xclass version "XCLASS_VERSION;
  info.copyright = "Copyright � 1998-2006, Hector Peraza.\n"
                   "Wavefront code by Nate Robins, 1997, 2000.";
  info.text = "This program is free software; you can redistribute it "
              "and/or modify it under the terms of the GNU "
              "General Public License.\n\n"
              "http://xclass.sourceforge.net\n"
              "http://www.pobox.com/~nate";

  new OXAboutDialog(_client->GetRoot(), this, &info);
}
