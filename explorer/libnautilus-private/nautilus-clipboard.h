/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* fm-directory-view.h
 *
 * Copyright (C) 1999, 2000  Free Software Foundaton
 * Copyright (C) 2000  Eazel, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Rebecca Schulman <rebecka@eazel.com>
 */

#ifndef NAUTILUS_CLIPBOARD_H
#define NAUTILUS_CLIPBOARD_H

#include <gtk/gtkeditable.h>
#include <gtk/gtktextview.h>
#include <gtk/gtkuimanager.h>

/* This makes this editable or text view put clipboard commands into
 * the passed UI manager when the editable/text view is in focus.
 * Callers in Nautilus normally get the UI manager from
 * nautilus_window_get_ui_manager. */
/* The shares selection changes argument should be set to true if the
 * widget uses the signal "selection_changed" to tell others about
 * text selection changes.  The NautilusEntry widget
 * is currently the only editable in nautilus that shares selection
 * changes. */
void nautilus_clipboard_set_up_editable            (GtkEditable        *target,
						    GtkUIManager       *ui_manager,
						    gboolean            shares_selection_changes);
void nautilus_clipboard_set_up_text_view           (GtkTextView        *target,
						    GtkUIManager       *ui_manager);


#endif /* NAUTILUS_CLIPBOARD_H */
