/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

typedef struct fentry_t
{
	/* scape list entry */
	char *name;
	char *file;

	/* associated widgets */
	GtkWidget *cb;		/* activate */
	GtkWidget *slider, *notb;	/* distance value, negate */
	GtkWidget *xstride, *ystride;	/* x and y stride */

	int disabled;
}
fentry_t;


#ifdef __cplusplus
extern "C"
{
#endif


GtkWidget *create_slider_entry(char *name, float min, float max, 
		GtkWidget **scalep, int show_widgets);

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
