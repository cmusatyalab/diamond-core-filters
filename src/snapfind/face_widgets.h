/*
 *
 *
 *                          Diamond 1.0
 * 
 *            Copyright (c) 2002-2004, Intel Corporation
 *                         All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Intel nor the names of its contributors may
 *      be used to endorse or promote products derived from this software 
 *      without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


typedef struct fentry_t {
  /* scape list entry */
  char *name;
  char *file;

  /* associated widgets */
  GtkWidget *cb;		/* activate */
  GtkWidget *slider, *notb;	/* distance value, negate */
  GtkWidget *xstride, *ystride;	/* x and y stride */

  int disabled;
} fentry_t;


#ifdef __cplusplus
extern "C" {
#endif

	
GtkWidget *create_slider_entry(char *name, float min, float max, GtkWidget **scalep,
			       int show_widgets);

static const GtkAttachOptions FILL = (GtkAttachOptions)(GTK_EXPAND|GTK_SHRINK|GTK_FILL);
static const GtkAttachOptions NOFILL = (GtkAttachOptions)0;


void save_expert_widget(GtkWidget *widget);
void show_expert_widgets();
void hide_expert_widgets();

void add_scape_widgets1(GtkWidget *table, int row, fentry_t *scape);
void add_scape_widgets2(GtkWidget *table, int row, fentry_t *scape);

#ifdef __cplusplus
}
#endif
