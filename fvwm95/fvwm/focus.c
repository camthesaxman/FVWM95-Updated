/****************************************************************************
 * This module is all original code 
 * by Rob Nation 
 * Copyright 1993, Robert Nation
 *     You may use this code for any purpose, as long as the original
 *     copyright remains in the source code and all documentation
 ****************************************************************************/

/***********************************************************************
 *
 * fvwm focus-setting code
 *
 ***********************************************************************/

#include <FVWMconfig.h>

#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "fvwm.h"
#include "menus.h"
#include "misc.h"
#include "parse.h"
#include "screen.h"
#include "module.h"


/********************************************************************
 *
 * Sets the input focus to the indicated window.
 *
 **********************************************************************/

void SetFocus(Window w, FvwmWindow *Fw, Bool FocusByMouse)
{
  int i;
  extern Time lastTimestamp;

  /* ClickToFocus focus queue manipulation - only performed for
   * Focus-by-mouse type focus events */
  if ((FocusByMouse) && (Fw && Fw != Scr.Focus && Fw != &Scr.FvwmRoot)) {
    FvwmWindow *tmp_win1, *tmp_win2;

    tmp_win1 = Fw->prev;
    tmp_win2 = Fw->next;

    if (tmp_win1)
      tmp_win1->next = tmp_win2;
    if (tmp_win2)
      tmp_win2->prev = tmp_win1;

    Fw->next = Scr.FvwmRoot.next;
    if (Scr.FvwmRoot.next)
      Scr.FvwmRoot.next->prev = Fw;
    Scr.FvwmRoot.next = Fw;
    Fw->prev = &Scr.FvwmRoot;
  }

  if (Scr.NumberOfScreens > 1) {
    XQueryPointer(dpy, Scr.Root, &JunkRoot, &JunkChild,
		  &JunkX, &JunkY, &JunkX, &JunkY, &JunkMask);
    if (JunkRoot != Scr.Root) {
      if ((Scr.Ungrabbed != NULL) && (Scr.Ungrabbed->flags & ClickToFocus)) {
	/* Need to grab buttons for focus window */
	XSync(dpy, 0);
	for (i = 0; i < 3; i++)
	  if (Scr.buttons2grab & (1 << i)) {
	    XGrabButton(dpy, (i + 1), 0, Scr.Ungrabbed->frame, True,
			ButtonPressMask, GrabModeSync, GrabModeAsync,
			None, Scr.FvwmCursors[SYS]);
	    XGrabButton(dpy, (i + 1), LockMask, Scr.Ungrabbed->frame, True,
			ButtonPressMask, GrabModeSync, GrabModeAsync,
			None, Scr.FvwmCursors[SYS]);
	  }
	Scr.Focus = NULL;
	Scr.Ungrabbed = NULL;
	XSetInputFocus(dpy, Scr.NoFocusWin, RevertToParent, lastTimestamp);
      }
      return;
    }
  }

  if ((Fw != NULL) && (Fw->Desk != Scr.CurrentDesk)) {
    Fw = NULL;
    w = Scr.NoFocusWin;
  }

  if ((Scr.Ungrabbed != NULL) && (Scr.Ungrabbed->flags & ClickToFocus)
      && (Scr.Ungrabbed != Fw)) {
    /* need to grab all buttons for window that we are about to
     * unfocus */
    XSync(dpy, 0);
    for (i = 0; i < 3; i++)
      if (Scr.buttons2grab & (1 << i))
	XGrabButton(dpy, (i + 1), 0, Scr.Ungrabbed->frame, True,
		    ButtonPressMask, GrabModeSync, GrabModeAsync, None,
		    Scr.FvwmCursors[SYS]);
    Scr.Ungrabbed = NULL;
  }
  /* if we do click to focus, remove the grab on mouse events that
   * was made to detect the focus change */
  if ((Fw != NULL) && (Fw->flags & ClickToFocus)) {
    for (i = 0; i < 3; i++)
      if (Scr.buttons2grab & (1 << i)) {
	XUngrabButton(dpy, (i + 1), 0, Fw->frame);
	XUngrabButton(dpy, (i + 1), LockMask, Fw->frame);
      }
    Scr.Ungrabbed = Fw;
  }
  if ((Fw) && (Fw->flags & ICONIFIED) && (Fw->icon_w))
    w = Fw->icon_w;

  if ((Fw) && (Fw->flags & Lenience)) {
    XSetInputFocus(dpy, w, RevertToParent, lastTimestamp);
    Scr.Focus = Fw;
    Scr.UnknownWinFocused = None;
  }
    else if (!((Fw) && (Fw->wmhints) && (Fw->wmhints->flags & InputHint) &&
	       (Fw->wmhints->input == False))) {
    /* Window will accept input focus */
    XSetInputFocus(dpy, w, RevertToParent, lastTimestamp);
    Scr.Focus = Fw;
    Scr.UnknownWinFocused = None;
  } else if ((Scr.Focus) && (Scr.Focus->Desk == Scr.CurrentDesk)) {
    /* Window doesn't want focus. Leave focus alone */
    /* XSetInputFocus (dpy,Scr.Hilite->w , RevertToParent, lastTimestamp); */
  } else {
    XSetInputFocus(dpy, Scr.NoFocusWin, RevertToParent, lastTimestamp);
    Scr.Focus = NULL;
  }


  if ((Fw) && (Fw->flags & DoesWmTakeFocus))
    send_clientmessage(w, _XA_WM_TAKE_FOCUS, lastTimestamp);

  XSync(dpy, 0);
}
